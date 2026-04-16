import { useEffect, useMemo } from 'react';
import { useApp } from '../context/AppContext';
import { useChart } from '../hooks/useChart';
import { makeChartOpt, calcStats } from '../utils/charts';

function getBadge(val: number | null, low: number, warnH: number, high: number) {
  if (val == null) return { cls: 's-badge', text: '--' };
  if (val < low) return { cls: 's-badge b-low', text: '偏低' };
  if (val > high) return { cls: 's-badge b-high', text: '偏高' };
  if (val > warnH) return { cls: 's-badge b-warn', text: '注意' };
  return { cls: 's-badge b-ok', text: '正常' };
}

function fmtUptime(s: number) {
  const d = Math.floor(s / 86400), h = Math.floor((s % 86400) / 3600), m = Math.floor((s % 3600) / 60), sec = s % 60;
  if (d > 0) return `${d}天 ${h}时 ${m}分`;
  if (h > 0) return `${h}时 ${m}分 ${sec}秒`;
  return `${m}分 ${sec}秒`;
}

const SENSORS = [
  { key: 'temp' as const, icon: '🌡️', label: '温度', unit: '°C', cls: 'sc-temp', dec: 1, low: 10, warnH: 28, high: 35 },
  { key: 'hum' as const, icon: '💧', label: '湿度', unit: '%', cls: 'sc-hum', dec: 1, low: 20, warnH: 80, high: 90 },
  { key: 'lux' as const, icon: '☀️', label: '光照', unit: ' lx', cls: 'sc-lux', dec: 0, low: 200, warnH: 800, high: 2000 },
  { key: 'soil' as const, icon: '🌱', label: '土壤水分', unit: '%', cls: 'sc-soil', dec: 0, low: 30, warnH: 60, high: 80 },
  { key: 'eco2' as const, icon: '☁️', label: 'eCO2', unit: ' ppm', cls: 'sc-co2', dec: 0, low: 400, warnH: 1000, high: 1500 },
  { key: 'tvoc' as const, icon: '🧪', label: 'TVOC', unit: ' ppb', cls: 'sc-tvoc', dec: 0, low: 0, warnH: 220, high: 660 },
];

const STATS_METRICS = [
  { key: 'temp' as const, label: '🌡️ 温度', unit: '°C', dec: 1 },
  { key: 'hum' as const, label: '💧 湿度', unit: '%', dec: 1 },
  { key: 'lux' as const, label: '☀️ 光照', unit: 'lx', dec: 0 },
  { key: 'soil' as const, label: '🌱 土壤', unit: '%', dec: 0 },
  { key: 'eco2' as const, label: '☁️ eCO2', unit: 'ppm', dec: 0 },
  { key: 'tvoc' as const, label: '🧪 TVOC', unit: 'ppb', dec: 0 },
];

export default function Dashboard() {
  const { state } = useApp();
  const { current, history, time } = state;
  const isAuto = state.state.mode === 'auto';

  const thChart = useChart(makeChartOpt('温湿度', ['温度(°C)', '湿度(%)'], ['#ef4444', '#3b82f6'], ['°C', '%']));
  const lsChart = useChart(makeChartOpt('光照·土壤', ['光照(lx)', '土壤(%)'], ['#f97316', '#22c55e'], ['lx', '%']));
  const aqChart = useChart(makeChartOpt('空气质量', ['eCO2(ppm)', 'TVOC(ppb)'], ['#14b8a6', '#a855f7'], ['ppm', 'ppb'], 400));

  useEffect(() => {
    if (!history.time.length) return;
    thChart.updateData({ xAxis: { data: history.time }, series: [{ data: history.temp }, { data: history.hum }] });
    lsChart.updateData({ xAxis: { data: history.time }, series: [{ data: history.lux }, { data: history.soil }] });
    aqChart.updateData({ xAxis: { data: history.time }, series: [{ data: history.eco2 }, { data: history.tvoc }] });
  }, [history, thChart, lsChart, aqChart]);

  const alerts = useMemo(() => {
    const c = current;
    const a: { cls: string; msg: string }[] = [];
    if (c.temp != null) {
      if (c.temp > 35) a.push({ cls: 'alert-danger', msg: `⚠️ 温度过高 (${c.temp.toFixed(1)}°C)` });
      else if (c.temp < 10) a.push({ cls: 'alert-danger', msg: `🥶 温度过低 (${c.temp.toFixed(1)}°C)` });
      else if (c.temp > 28) a.push({ cls: 'alert-warn', msg: `🌡️ 温度偏高 (${c.temp.toFixed(1)}°C)` });
      else if (c.temp < 15) a.push({ cls: 'alert-warn', msg: `🌡️ 温度偏低 (${c.temp.toFixed(1)}°C)` });
    }
    if (c.hum != null) {
      if (c.hum > 90) a.push({ cls: 'alert-danger', msg: `💧 湿度过高 (${c.hum.toFixed(1)}%)` });
      else if (c.hum < 20) a.push({ cls: 'alert-warn', msg: `💧 湿度偏低 (${c.hum.toFixed(1)}%)` });
    }
    if (c.soil != null) {
      if (c.soil < 20) a.push({ cls: 'alert-danger', msg: `🌱 土壤严重缺水 (${c.soil}%)` });
      else if (c.soil < 30) a.push({ cls: 'alert-warn', msg: `🌱 土壤偏干 (${c.soil}%)` });
    }
    if (c.eco2 != null) {
      if (c.eco2 > 1500) a.push({ cls: 'alert-danger', msg: `☁️ CO2超标 (${c.eco2}ppm)` });
      else if (c.eco2 > 1000) a.push({ cls: 'alert-warn', msg: `☁️ CO2偏高 (${c.eco2}ppm)` });
    }
    if (c.tvoc != null) {
      if (c.tvoc > 660) a.push({ cls: 'alert-danger', msg: `🧪 TVOC超标 (${c.tvoc}ppb)` });
      else if (c.tvoc > 220) a.push({ cls: 'alert-warn', msg: `🧪 TVOC偏高 (${c.tvoc}ppb)` });
    }
    return a;
  }, [current]);

  const statsRows = useMemo(() => {
    return STATS_METRICS.map((m) => {
      const arr = history[m.key] as number[];
      const s = calcStats(arr);
      const cur = current[m.key];
      return { ...m, stats: s, cur };
    }).filter((r) => r.stats);
  }, [history, current]);

  return (
    <div>
      <div className="flex items-center justify-between mb-5">
        <div>
          <h1 className="text-xl font-bold text-gray-800">实时监控</h1>
          <p className="text-xs text-gray-400 mt-1">系统时间: {time || '--'}</p>
        </div>
        <span className={`text-xs px-3 py-1 rounded-full font-medium ${isAuto ? 'bg-green-50 text-green-600' : 'bg-orange-50 text-orange-600'}`}>
          {isAuto ? '自动模式' : '手动模式'}
        </span>
      </div>

      {/* Sensor cards */}
      <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4 mb-6">
        {SENSORS.map((s) => {
          const val = current[s.key];
          const badge = getBadge(val, s.low, s.warnH, s.high);
          return (
            <div key={s.key} className={`card card-sm sensor-card ${s.cls}`}>
              <div className="flex items-center justify-between">
                <span className="sensor-icon">{s.icon}</span>
                <span className={badge.cls}>{badge.text}</span>
              </div>
              <div className="sensor-val">
                {val != null ? val.toFixed(s.dec) + s.unit : `--${s.unit}`}
              </div>
              <div className="sensor-label">{s.label}</div>
            </div>
          );
        })}
      </div>

      {/* Alerts */}
      <div className="card mb-6">
        <h3 className="text-sm font-bold text-gray-600 mb-3">⚡ 环境预警</h3>
        <div>
          {alerts.length === 0 ? (
            <div className="text-green-600 text-sm font-medium">✅ 环境状态良好</div>
          ) : (
            alerts.map((a, i) => <div key={i} className={`alert-item ${a.cls}`}>{a.msg}</div>)
          )}
        </div>
      </div>

      {/* Real-time charts */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
        <div className="card card-sm"><div className="chart-box" ref={thChart.containerRef} /></div>
        <div className="card card-sm"><div className="chart-box" ref={lsChart.containerRef} /></div>
        <div className="card card-sm"><div className="chart-box" ref={aqChart.containerRef} /></div>
      </div>

      {/* Stats table */}
      <div className="card">
        <h3 className="text-sm font-bold text-gray-600 mb-3">📈 实时统计</h3>
        <div className="overflow-x-auto">
          <table className="w-full text-sm text-center">
            <thead>
              <tr className="bg-slate-50 text-gray-500 text-xs">
                <th className="px-3 py-2 text-left font-semibold">指标</th>
                <th className="px-3 py-2 font-semibold">最小</th>
                <th className="px-3 py-2 font-semibold">最大</th>
                <th className="px-3 py-2 font-semibold">平均</th>
                <th className="px-3 py-2 font-semibold">当前</th>
              </tr>
            </thead>
            <tbody>
              {statsRows.map((r) => (
                <tr key={r.key} className="border-b border-slate-100 hover:bg-slate-50">
                  <td className="px-3 py-2 font-medium text-gray-700 text-left text-xs">{r.label}</td>
                  <td className="px-3 py-2 text-blue-600 font-mono text-xs">{r.stats!.min.toFixed(r.dec)}</td>
                  <td className="px-3 py-2 text-red-500 font-mono text-xs">{r.stats!.max.toFixed(r.dec)}</td>
                  <td className="px-3 py-2 text-gray-600 font-mono text-xs">{r.stats!.avg.toFixed(r.dec)}</td>
                  <td className="px-3 py-2 text-green-700 font-mono text-xs font-bold">
                    {r.cur != null ? `${r.cur.toFixed(r.dec)} ${r.unit}` : '--'}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Uptime footer (debug) */}
      {state.system && (
        <p className="text-xs text-gray-400 mt-4 text-right">
          运行: {fmtUptime(state.system.uptime)} · 内存: {((state.system.heap_total - state.system.heap_free) / state.system.heap_total * 100).toFixed(1)}%
        </p>
      )}
    </div>
  );
}
