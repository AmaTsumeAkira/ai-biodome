import { useState, useEffect, useCallback, useRef } from 'react';
import { useChart } from '../hooks/useChart';
import { makeChartOpt, calcStats } from '../utils/charts';

interface DateInfo {
  data_dates: string[];
  log_dates: string[];
  total_bytes?: number;
  used_bytes?: number;
}

interface HistoryApiData {
  time: string[];
  temp: number[];
  hum: number[];
  lux: number[];
  soil: number[];
  eco2: number[];
  tvoc: number[];
  error?: string;
}

const STATS = [
  { key: 'temp', label: '🌡️ 温度', unit: '°C', dec: 1 },
  { key: 'hum', label: '💧 湿度', unit: '%', dec: 1 },
  { key: 'lux', label: '☀️ 光照', unit: 'lx', dec: 0 },
  { key: 'soil', label: '🌱 土壤', unit: '%', dec: 0 },
  { key: 'eco2', label: '☁️ eCO2', unit: 'ppm', dec: 0 },
  { key: 'tvoc', label: '🧪 TVOC', unit: 'ppb', dec: 0 },
] as const;

export default function History() {
  const [dates, setDates] = useState<string[]>([]);
  const [selectedDate, setSelectedDate] = useState('');
  const [storageInfo, setStorageInfo] = useState('存储状态加载中...');
  const [interval, setInterval] = useState(300);
  const [intervalHint, setIntervalHint] = useState('');
  const [showCharts, setShowCharts] = useState(false);
  const [showStats, setShowStats] = useState(false);
  const [showLog, setShowLog] = useState(false);
  const [logEntries, setLogEntries] = useState<string[]>([]);
  const [histData, setHistData] = useState<HistoryApiData | null>(null);
  const chartsReady = useRef(false);

  const thChart = useChart(makeChartOpt('温湿度', ['温度(°C)', '湿度(%)'], ['#ef4444', '#3b82f6'], ['°C', '%']));
  const lsChart = useChart(makeChartOpt('光照·土壤', ['光照(lx)', '土壤(%)'], ['#f97316', '#22c55e'], ['lx', '%']));
  const aqChart = useChart(makeChartOpt('空气质量', ['eCO2(ppm)', 'TVOC(ppb)'], ['#14b8a6', '#a855f7'], ['ppm', 'ppb'], 400));

  // Load available dates
  useEffect(() => {
    const load = async () => {
      try {
        const res = await fetch('/api/dates');
        const d: DateInfo = await res.json();
        const all: Record<string, true> = {};
        (d.data_dates || []).forEach((x) => (all[x] = true));
        (d.log_dates || []).forEach((x) => (all[x] = true));
        const sorted = Object.keys(all).sort().reverse();
        setDates(sorted);
        if (sorted.length) setSelectedDate(sorted[0]);
        if (d.total_bytes) {
          setStorageInfo(
            `存储: ${((d.used_bytes || 0) / 1024).toFixed(1)}KB / ${(d.total_bytes / 1024).toFixed(0)}KB | 归档: ${sorted.length}天`,
          );
        }
      } catch { /* ignore */ }
    };
    load();
  }, []);

  // Load recording interval
  useEffect(() => {
    fetch('/api/interval')
      .then((r) => r.json())
      .then((d: { interval: number }) => {
        if (d.interval) {
          setInterval(d.interval);
          setIntervalHint(`当前: ${d.interval}秒 (${(d.interval / 60).toFixed(1)}分钟)`);
        }
      })
      .catch(() => {});
  }, []);

  const fmtDate = (d: string) => `${d.substring(0, 4)}-${d.substring(4, 6)}-${d.substring(6, 8)}`;

  const loadHistoryData = useCallback(async () => {
    if (!selectedDate) return;
    try {
      const res = await fetch(`/api/history?date=${selectedDate}`);
      const data: HistoryApiData = await res.json();
      if (data.error) { alert(data.error); return; }
      setHistData(data);
      setShowCharts(true);
      setShowStats(true);
      setShowLog(false);
      chartsReady.current = true;

      const dl = fmtDate(selectedDate);
      thChart.resetOption(makeChartOpt(`${dl} 温湿度`, ['温度(°C)', '湿度(%)'], ['#ef4444', '#3b82f6'], ['°C', '%']));
      thChart.updateData({ xAxis: { data: data.time }, series: [{ data: data.temp }, { data: data.hum }] });
      lsChart.resetOption(makeChartOpt(`${dl} 光照·土壤`, ['光照(lx)', '土壤(%)'], ['#f97316', '#22c55e'], ['lx', '%']));
      lsChart.updateData({ xAxis: { data: data.time }, series: [{ data: data.lux }, { data: data.soil }] });
      aqChart.resetOption(makeChartOpt(`${dl} 空气质量`, ['eCO2(ppm)', 'TVOC(ppb)'], ['#14b8a6', '#a855f7'], ['ppm', 'ppb'], 400));
      aqChart.updateData({ xAxis: { data: data.time }, series: [{ data: data.eco2 }, { data: data.tvoc }] });
    } catch (e) { alert('查询失败: ' + (e as Error).message); }
  }, [selectedDate, thChart, lsChart, aqChart]);

  const loadHistoryLog = useCallback(async () => {
    if (!selectedDate) return;
    try {
      const res = await fetch(`/api/log?date=${selectedDate}`);
      const data = await res.json();
      if (data.error) { alert(data.error); return; }
      setLogEntries(data.entries || []);
      setShowCharts(false);
      setShowStats(false);
      setShowLog(true);
    } catch (e) { alert('查询失败: ' + (e as Error).message); }
  }, [selectedDate]);

  const saveSaveInterval = async () => {
    let val = interval;
    if (val < 10) val = 10;
    if (val > 3600) val = 3600;
    try {
      const res = await fetch('/api/interval', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ interval: val }),
      });
      const d = await res.json();
      if (d.interval != null) {
        alert(`记录粒度已设为 ${d.interval} 秒`);
        setIntervalHint(`当前: ${d.interval}秒 (${(d.interval / 60).toFixed(1)}分钟)`);
      }
    } catch (e) { alert('保存失败: ' + (e as Error).message); }
  };

  const [exportStart, setExportStart] = useState('');
  const [exportEnd, setExportEnd] = useState('');

  useEffect(() => {
    if (dates.length > 0) {
      setExportStart(dates[dates.length - 1]);
      setExportEnd(dates[0]);
    }
  }, [dates]);

  const exportCSV = () => {
    let s = exportStart, e = exportEnd;
    if (!s || !e) { alert('请选择日期范围'); return; }
    if (s > e) { [s, e] = [e, s]; }
    window.open(`/api/export?start=${s}&end=${e}`, '_blank');
  };

  return (
    <div>
      <h1 className="text-xl font-bold text-gray-800 mb-5">历史数据</h1>

      <div className="card mb-6">
        <div className="flex flex-wrap items-center gap-3 mb-3">
          <select
            className="px-3 py-2 rounded-lg border text-sm focus:ring-2 focus:ring-blue-400 outline-none"
            value={selectedDate}
            onChange={(e) => setSelectedDate(e.target.value)}
          >
            {dates.length === 0 && <option value="">暂无归档</option>}
            {dates.map((d) => (
              <option key={d} value={d}>{fmtDate(d)}</option>
            ))}
          </select>
          <button onClick={loadHistoryData} className="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">查询数据</button>
          <button onClick={loadHistoryLog} className="px-4 py-2 bg-gray-500 hover:bg-gray-600 text-white rounded-lg text-sm font-medium transition-colors">查看日志</button>
        </div>
        <div className="text-xs text-gray-400">{storageInfo}</div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-6">
        <div className="card card-sm">
          <h3 className="text-sm font-bold text-gray-600 mb-3">📊 记录粒度</h3>
          <div className="flex items-center gap-3">
            <input type="number" min={10} max={3600} step={10} value={interval} onChange={(e) => setInterval(+e.target.value)} className="w-24 px-2 py-1 rounded border text-sm text-center focus:ring-2 focus:ring-blue-400 outline-none" />
            <span className="text-sm text-gray-500">秒</span>
            <button onClick={saveSaveInterval} className="px-3 py-1 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-xs font-medium transition-colors">保存</button>
            <span className="text-xs text-gray-400">{intervalHint || '默认 300秒(5分钟)'}</span>
          </div>
        </div>
        <div className="card card-sm">
          <h3 className="text-sm font-bold text-gray-600 mb-3">📥 数据导出</h3>
          <div className="flex flex-wrap items-center gap-2">
            <select className="px-2 py-1 rounded border text-sm" value={exportStart} onChange={(e) => setExportStart(e.target.value)}>
              {dates.map((d) => <option key={d} value={d}>{fmtDate(d)}</option>)}
            </select>
            <span className="text-gray-400 text-sm">至</span>
            <select className="px-2 py-1 rounded border text-sm" value={exportEnd} onChange={(e) => setExportEnd(e.target.value)}>
              {dates.map((d) => <option key={d} value={d}>{fmtDate(d)}</option>)}
            </select>
            <button onClick={exportCSV} className="px-3 py-1 bg-green-500 hover:bg-green-600 text-white rounded-lg text-xs font-medium transition-colors">导出 CSV</button>
          </div>
        </div>
      </div>

      {/* History charts */}
      {showCharts && (
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
          <div className="card card-sm"><div className="chart-box" ref={thChart.containerRef} /></div>
          <div className="card card-sm"><div className="chart-box" ref={lsChart.containerRef} /></div>
          <div className="card card-sm"><div className="chart-box" ref={aqChart.containerRef} /></div>
        </div>
      )}

      {/* History stats */}
      {showStats && histData && (
        <div className="card mb-6">
          <h3 className="text-sm font-bold text-gray-600 mb-3">当日统计摘要</h3>
          <div className="overflow-x-auto">
            <table className="w-full text-sm text-center">
              <thead>
                <tr className="bg-slate-50 text-gray-500 text-xs">
                  <th className="px-3 py-2 text-left font-semibold">指标</th>
                  <th className="px-3 py-2 font-semibold">最小</th>
                  <th className="px-3 py-2 font-semibold">最大</th>
                  <th className="px-3 py-2 font-semibold">平均</th>
                  <th className="px-3 py-2 font-semibold">数据点</th>
                </tr>
              </thead>
              <tbody>
                {STATS.map((m) => {
                  const arr = histData[m.key as keyof HistoryApiData] as number[];
                  const s = calcStats(arr);
                  if (!s) return null;
                  return (
                    <tr key={m.key} className="border-b border-slate-100">
                      <td className="px-3 py-2 text-left text-xs font-medium text-gray-700">{m.label}</td>
                      <td className="px-3 py-2 text-blue-600 font-mono text-xs">{s.min.toFixed(m.dec)}</td>
                      <td className="px-3 py-2 text-red-500 font-mono text-xs">{s.max.toFixed(m.dec)}</td>
                      <td className="px-3 py-2 text-gray-600 font-mono text-xs">{s.avg.toFixed(m.dec)}</td>
                      <td className="px-3 py-2 font-mono text-xs">{arr.length}</td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        </div>
      )}

      {/* Log viewer */}
      {showLog && (
        <div className="card">
          <h3 className="text-sm font-bold text-gray-600 mb-3">操作日志</h3>
          <div className="bg-slate-50 rounded-lg p-4 max-h-80 overflow-y-auto text-xs font-mono text-gray-700 space-y-1">
            {logEntries.length === 0 ? (
              <div className="text-gray-400">当日无记录</div>
            ) : (
              logEntries.map((entry, i) => (
                <div key={i} className="py-1 border-b border-slate-200">
                  {entry.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')}
                </div>
              ))
            )}
          </div>
        </div>
      )}
    </div>
  );
}
