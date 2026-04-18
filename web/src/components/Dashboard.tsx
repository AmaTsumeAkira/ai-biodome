import { useMemo } from 'react';
import { Card, Row, Col, Tag, Alert, Table, Typography, Flex } from 'antd';
import { Line } from '@ant-design/plots';
import { useApp } from '../context/AppContext';
import { calcStats, toChartData } from '../utils/charts';

const { Title, Text } = Typography;

const SENSORS = [
  { key: 'temp' as const, label: '🌡️ 温度', unit: '°C', dec: 1, low: 10, warnH: 28, high: 35, color: '#ef4444' },
  { key: 'hum' as const, label: '💧 湿度', unit: '%', dec: 1, low: 20, warnH: 80, high: 90, color: '#0ea5e9' },
  { key: 'lux' as const, label: '☀️ 光照', unit: 'lx', dec: 0, low: 200, warnH: 800, high: 2000, color: '#f59e0b' },
  { key: 'soil' as const, label: '🌱 土壤水分', unit: '%', dec: 0, low: 30, warnH: 60, high: 80, color: '#22c55e' },
  { key: 'eco2' as const, label: '☁️ eCO2', unit: 'ppm', dec: 0, low: 400, warnH: 1000, high: 1500, color: '#06b6d4' },
  { key: 'tvoc' as const, label: '🧪 TVOC', unit: 'ppb', dec: 0, low: 0, warnH: 220, high: 660, color: '#8b5cf6' },
];

function getBadge(val: number | null, low: number, warnH: number, high: number) {
  if (val == null) return { color: 'default' as const, text: 'N/A', led: '#64748b' };
  if (val < low) return { color: 'blue' as const, text: 'LOW', led: '#0ea5e9' };
  if (val > high) return { color: 'red' as const, text: 'HIGH', led: '#ef4444' };
  if (val > warnH) return { color: 'orange' as const, text: 'WARN', led: '#f59e0b' };
  return { color: 'green' as const, text: 'OK', led: '#22c55e' };
}

function fmtUptime(s: number) {
  const d = Math.floor(s / 86400), h = Math.floor((s % 86400) / 3600), m = Math.floor((s % 3600) / 60), sec = s % 60;
  if (d > 0) return `${d}天 ${h}时 ${m}分`;
  if (h > 0) return `${h}时 ${m}分 ${sec}秒`;
  return `${m}分 ${sec}秒`;
}

const statsColumns = [
  { title: '指标', dataIndex: 'label', key: 'label', width: 100 },
  { title: '最小', dataIndex: 'min', key: 'min', align: 'center' as const, render: (v: string) => <Text type="secondary">{v}</Text> },
  { title: '最大', dataIndex: 'max', key: 'max', align: 'center' as const, render: (v: string) => <Text type="danger">{v}</Text> },
  { title: '平均', dataIndex: 'avg', key: 'avg', align: 'center' as const },
  { title: '当前', dataIndex: 'cur', key: 'cur', align: 'center' as const, render: (v: string) => <Text strong>{v}</Text> },
];

const lineConfig = (colors: string[]) => ({
  xField: 'time' as const,
  yField: 'value' as const,
  colorField: 'type' as const,
  smooth: false,
  height: 220,
  axis: {
    x: { label: { autoRotate: false, style: { fontSize: 10, fill: '#94a3b8' } }, line: { style: { stroke: '#e2e8f0' } } },
    y: { title: false as const, label: { style: { fontSize: 10, fill: '#94a3b8' } }, grid: { line: { style: { stroke: '#f1f5f9', lineDash: [3, 3] } } } },
  },
  style: { lineWidth: 1.5 },
  scale: { color: { range: colors } },
  legend: { position: 'top' as const, itemMarker: 'smooth' as const, itemLabelFontSize: 11 },
  tooltip: { channel: 'y' as const },
  animation: false,
});

export default function Dashboard() {
  const { state } = useApp();
  const { current, history, time } = state;
  const isAuto = state.state.mode === 'auto';

  const thData = useMemo(() => toChartData(history.time, [
    { data: history.temp, label: '温度(°C)' },
    { data: history.hum, label: '湿度(%)' },
  ]), [history]);

  const lsData = useMemo(() => toChartData(history.time, [
    { data: history.lux, label: '光照(lx)' },
    { data: history.soil, label: '土壤(%)' },
  ]), [history]);

  const aqData = useMemo(() => toChartData(history.time, [
    { data: history.eco2, label: 'eCO2(ppm)' },
    { data: history.tvoc, label: 'TVOC(ppb)' },
  ]), [history]);

  const alerts = useMemo(() => {
    const c = current;
    const a: { type: 'error' | 'warning'; msg: string }[] = [];
    if (c.temp != null) {
      if (c.temp > 35) a.push({ type: 'error', msg: `🔥 温度过高 (${c.temp.toFixed(1)}°C > 35°C)` });
      else if (c.temp < 10) a.push({ type: 'error', msg: `🥶 温度过低 (${c.temp.toFixed(1)}°C < 10°C)` });
      else if (c.temp > 28) a.push({ type: 'warning', msg: `🌡️ 温度偏高 (${c.temp.toFixed(1)}°C > 28°C)` });
    }
    if (c.hum != null && c.hum > 90) a.push({ type: 'error', msg: `💧 湿度过高 (${c.hum.toFixed(1)}% > 90%)` });
    if (c.soil != null) {
      if (c.soil < 20) a.push({ type: 'error', msg: `🌱 土壤严重缺水 (${c.soil}% < 20%)` });
      else if (c.soil < 30) a.push({ type: 'warning', msg: `🌱 土壤偏干 (${c.soil}% < 30%)` });
    }
    if (c.eco2 != null && c.eco2 > 1500) a.push({ type: 'error', msg: `☁️ CO₂超标 (${c.eco2}ppm > 1500ppm)` });
    if (c.tvoc != null && c.tvoc > 660) a.push({ type: 'error', msg: `🧪 TVOC超标 (${c.tvoc}ppb > 660ppb)` });
    return a;
  }, [current]);

  const statsData = useMemo(() => {
    const metrics = [
      { key: 'temp' as const, label: '🌡️ 温度', unit: '°C', dec: 1 },
      { key: 'hum' as const, label: '💧 湿度', unit: '%', dec: 1 },
      { key: 'lux' as const, label: '☀️ 光照', unit: 'lx', dec: 0 },
      { key: 'soil' as const, label: '🌱 土壤', unit: '%', dec: 0 },
      { key: 'eco2' as const, label: '☁️ eCO2', unit: 'ppm', dec: 0 },
      { key: 'tvoc' as const, label: '🧪 TVOC', unit: 'ppb', dec: 0 },
    ];
    return metrics
      .map((m) => {
        const arr = history[m.key] as number[];
        const s = calcStats(arr);
        if (!s) return null;
        const cur = current[m.key];
        return {
          key: m.key,
          label: m.label,
          min: s.min.toFixed(m.dec),
          max: s.max.toFixed(m.dec),
          avg: s.avg.toFixed(m.dec),
          cur: cur != null ? `${cur.toFixed(m.dec)} ${m.unit}` : '--',
        };
      })
      .filter(Boolean);
  }, [history, current]);

  return (
    <>
      <div className="page-header">
        <Flex justify="space-between" align="center">
          <div>
            <Title level={4} style={{ margin: 0, color: '#1e293b', letterSpacing: 0.5, fontWeight: 700 }}>📊 实时监控</Title>
            <Text type="secondary" style={{ fontSize: 11 }} className="ind-value">{time || '--:--:--'}</Text>
          </div>
          <Tag
            color={isAuto ? 'success' : 'warning'}
            className={isAuto ? 'status-tag-ok' : 'status-tag-warn'}
            style={{ fontSize: 12, padding: '2px 12px', fontWeight: 700, letterSpacing: 0.5 }}
          >
            {isAuto ? 'AUTO' : 'MANUAL'}
          </Tag>
        </Flex>
      </div>

      {/* Sensor panels */}
      <Row gutter={[8, 8]} style={{ marginBottom: 16 }}>
        {SENSORS.map((s) => {
          const val = current[s.key];
          const badge = getBadge(val, s.low, s.warnH, s.high);
          const pct = val != null ? Math.min(100, Math.max(0, (val - s.low) / (s.high - s.low) * 100)) : 0;
          return (
            <Col xs={12} md={8} lg={4} key={s.key}>
              <Card
                size="small"
                className="sensor-card"
                style={{ borderLeft: `3px solid ${s.color}`, borderRadius: 4, boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}
                styles={{ body: { padding: '10px 12px' } }}
              >
                <Flex justify="space-between" align="center" style={{ marginBottom: 2 }}>
                  <Text style={{ fontSize: 11, color: '#64748b', fontWeight: 700, textTransform: 'uppercase' as const, letterSpacing: 0.5 }}>{s.label}</Text>
                  <Flex align="center" gap={4}>
                    <span className="ind-led" style={{ color: badge.led }} />
                    <Tag
                      color={badge.color}
                      className={badge.text === 'OK' ? 'status-tag-ok' : badge.text === 'WARN' ? 'status-tag-warn' : badge.text === 'HIGH' ? 'status-tag-error' : ''}
                      style={{ margin: 0, fontSize: 10, lineHeight: '16px', padding: '0 4px', fontWeight: 600 }}
                    >{badge.text}</Tag>
                  </Flex>
                </Flex>
                <div className="ind-value" style={{ fontSize: 28, fontWeight: 700, color: '#1e293b', lineHeight: 1.2 }}>
                  {val != null ? val.toFixed(s.dec) : '--'}
                  <span style={{ fontSize: 12, fontWeight: 400, color: '#94a3b8', marginLeft: 2 }}>{s.unit}</span>
                </div>
                <div className="gauge-bar">
                  <div className="gauge-bar-fill" style={{ width: `${pct}%`, background: `linear-gradient(90deg, ${s.color}88, ${s.color})` }} />
                </div>
              </Card>
            </Col>
          );
        })}
      </Row>

      {/* Alerts */}
      <Card size="small" title="⚠️ 环境预警" className="ind-card" style={{ marginBottom: 16, borderLeft: alerts.length > 0 ? '3px solid #f59e0b' : '3px solid #22c55e' }}>
        {alerts.length === 0 ? (
          <Flex align="center" gap={8} style={{ padding: '4px 0' }}>
            <span className="ind-led" style={{ color: '#22c55e' }} />
            <Text strong className="ind-value" style={{ color: '#22c55e', fontSize: 13 }}>✅ 所有指标正常</Text>
          </Flex>
        ) : (
          <Flex vertical gap={6}>
            {alerts.map((a, i) => (
              <Alert key={i} type={a.type} message={<span className="ind-value" style={{ fontSize: 12 }}>{a.msg}</span>} showIcon banner className="ind-alert" />
            ))}
          </Flex>
        )}
      </Card>

      {/* Charts */}
      <Row gutter={[8, 8]} style={{ marginBottom: 16 }}>
        <Col xs={24} lg={8}>
          <Card size="small" title="🌡️ 温湿度趋势" className="ind-card">
            <div className="chart-container">
              {thData.length > 0 ? <Line data={thData} {...lineConfig(['#ef4444', '#0ea5e9'])} /> : <Text type="secondary">等待数据...</Text>}
            </div>
          </Card>
        </Col>
        <Col xs={24} lg={8}>
          <Card size="small" title="☀️ 光照 · 土壤" className="ind-card">
            <div className="chart-container">
              {lsData.length > 0 ? <Line data={lsData} {...lineConfig(['#f59e0b', '#22c55e'])} /> : <Text type="secondary">等待数据...</Text>}
            </div>
          </Card>
        </Col>
        <Col xs={24} lg={8}>
          <Card size="small" title="☁️ 空气质量" className="ind-card">
            <div className="chart-container">
              {aqData.length > 0 ? <Line data={aqData} {...lineConfig(['#06b6d4', '#8b5cf6'])} /> : <Text type="secondary">等待数据...</Text>}
            </div>
          </Card>
        </Col>
      </Row>

      {/* Stats table */}
      <Card size="small" title="📈 实时统计" className="ind-card">
        <Table
          className="ind-table"
          dataSource={statsData as any[]}
          columns={statsColumns}
          size="small"
          pagination={false}
          scroll={{ x: 400 }}
        />
      </Card>

      {state.system && (
        <Text type="secondary" className="ind-value" style={{ display: 'block', textAlign: 'right', marginTop: 8, fontSize: 11 }}>
          UPTIME: {fmtUptime(state.system.uptime)} | MEM: {((state.system.heap_total - state.system.heap_free) / state.system.heap_total * 100).toFixed(1)}%
        </Text>
      )}
    </>
  );
}
