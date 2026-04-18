import { useState, useEffect, useCallback } from 'react';
import { Card, Row, Col, Select, Button, InputNumber, Table, Typography, Flex, List, App } from 'antd';
import { SearchOutlined, FileTextOutlined, DownloadOutlined, SaveOutlined } from '@ant-design/icons';
import { Line } from '@ant-design/plots';
import { calcStats, toChartData } from '../utils/charts';

const { Title, Text } = Typography;

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

const statsColumns = [
  { title: '指标', dataIndex: 'label', key: 'label', width: 100 },
  { title: '最小', dataIndex: 'min', key: 'min', align: 'center' as const },
  { title: '最大', dataIndex: 'max', key: 'max', align: 'center' as const },
  { title: '平均', dataIndex: 'avg', key: 'avg', align: 'center' as const },
  { title: '数据点', dataIndex: 'count', key: 'count', align: 'center' as const },
];

const lineConfig = (colors: string[]) => ({
  xField: 'time' as const,
  yField: 'value' as const,
  colorField: 'type' as const,
  smooth: false,
  height: 220,
  axis: {
    x: { label: { autoRotate: false, style: { fontSize: 10, fill: '#94a3b8' } } },
    y: { title: false as const, label: { style: { fontSize: 10, fill: '#94a3b8' } } },
  },
  style: { lineWidth: 1.5 },
  scale: { color: { range: colors } },
  legend: { position: 'top' as const, itemMarker: 'smooth' as const },
  animation: false,
});

export default function History() {
  const { message } = App.useApp();
  const [dates, setDates] = useState<string[]>([]);
  const [selectedDate, setSelectedDate] = useState('');
  const [storageInfo, setStorageInfo] = useState('加载中...');
  const [saveInterval, setSaveInterval] = useState(300);
  const [showCharts, setShowCharts] = useState(false);
  const [showLog, setShowLog] = useState(false);
  const [logEntries, setLogEntries] = useState<string[]>([]);
  const [histData, setHistData] = useState<HistoryApiData | null>(null);
  const [loading, setLoading] = useState(false);
  const [exportStart, setExportStart] = useState('');
  const [exportEnd, setExportEnd] = useState('');

  const fmtDate = (d: string) => `${d.substring(0, 4)}-${d.substring(4, 6)}-${d.substring(6, 8)}`;

  useEffect(() => {
    fetch('/api/dates')
      .then((r) => r.json())
      .then((d: DateInfo) => {
        const all: Record<string, true> = {};
        (d.data_dates || []).forEach((x) => (all[x] = true));
        (d.log_dates || []).forEach((x) => (all[x] = true));
        const sorted = Object.keys(all).sort().reverse();
        setDates(sorted);
        if (sorted.length) {
          setSelectedDate(sorted[0]);
          setExportStart(sorted[sorted.length - 1]);
          setExportEnd(sorted[0]);
        }
        if (d.total_bytes) {
          setStorageInfo(`${((d.used_bytes || 0) / 1024).toFixed(1)}KB / ${(d.total_bytes / 1024).toFixed(0)}KB | 归档 ${sorted.length} 天`);
        }
      })
      .catch(() => setStorageInfo('无法获取'));

    fetch('/api/interval')
      .then((r) => r.json())
      .then((d: { interval: number }) => { if (d.interval) setSaveInterval(d.interval); })
      .catch(() => {});
  }, []);

  const loadData = useCallback(async () => {
    if (!selectedDate) return;
    setLoading(true);
    try {
      const res = await fetch(`/api/history?date=${selectedDate}`);
      const data: HistoryApiData = await res.json();
      if (data.error) { message.error(data.error); return; }
      setHistData(data);
      setShowCharts(true);
      setShowLog(false);
    } catch (e) {
      message.error('查询失败: ' + (e as Error).message);
    } finally {
      setLoading(false);
    }
  }, [selectedDate, message]);

  const loadLog = useCallback(async () => {
    if (!selectedDate) return;
    setLoading(true);
    try {
      const res = await fetch(`/api/log?date=${selectedDate}`);
      const data = await res.json();
      if (data.error) { message.error(data.error); return; }
      setLogEntries(data.entries || []);
      setShowCharts(false);
      setShowLog(true);
    } catch (e) {
      message.error('查询失败: ' + (e as Error).message);
    } finally {
      setLoading(false);
    }
  }, [selectedDate, message]);

  const doSaveInterval = async () => {
    let val = saveInterval;
    if (val < 10) val = 10;
    if (val > 3600) val = 3600;
    try {
      const res = await fetch('/api/interval', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ interval: val }),
      });
      const d = await res.json();
      if (d.interval != null) message.success(`记录粒度已设为 ${d.interval} 秒`);
    } catch (e) {
      message.error('保存失败: ' + (e as Error).message);
    }
  };

  const exportCSV = () => {
    let s = exportStart, e = exportEnd;
    if (!s || !e) { message.warning('请选择日期范围'); return; }
    if (s > e) [s, e] = [e, s];
    window.open(`/api/export?start=${s}&end=${e}`, '_blank');
  };

  const dateOpts = dates.map((d) => ({ value: d, label: fmtDate(d) }));

  const statsData = histData
    ? STATS.map((m) => {
        const arr = histData[m.key as keyof HistoryApiData] as number[];
        const s = calcStats(arr);
        if (!s) return null;
        return { key: m.key, label: m.label, min: s.min.toFixed(m.dec), max: s.max.toFixed(m.dec), avg: s.avg.toFixed(m.dec), count: arr.length };
      }).filter(Boolean)
    : [];

  return (
    <>
      <div className="page-header">
        <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>📁 历史数据</Title>
      </div>

      <Card style={{ marginBottom: 16, boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
        <Flex wrap gap={12} align="center">
          <Select
            value={selectedDate || undefined}
            onChange={setSelectedDate}
            options={dateOpts}
            placeholder="选择日期"
            style={{ width: 160 }}
            notFoundContent="暂无归档"
          />
          <Button type="primary" icon={<SearchOutlined />} loading={loading} onClick={loadData}>查询数据</Button>
          <Button icon={<FileTextOutlined />} loading={loading} onClick={loadLog}>查看日志</Button>
          <Text type="secondary" style={{ fontSize: 12 }}>存储: {storageInfo}</Text>
        </Flex>
      </Card>

      <Row gutter={[16, 16]} style={{ marginBottom: 16 }}>
        <Col xs={24} md={12}>
          <Card size="small" title="记录粒度" className="ind-card">
            <Flex align="center" gap={8}>
              <InputNumber min={10} max={3600} step={10} value={saveInterval} onChange={(v) => setSaveInterval(v ?? 300)} style={{ width: 100 }} size="small" addonAfter="秒" />
              <Button size="small" type="primary" icon={<SaveOutlined />} onClick={doSaveInterval}>保存</Button>
              <Text type="secondary" style={{ fontSize: 12 }}>{(saveInterval / 60).toFixed(1)} 分钟/次</Text>
            </Flex>
          </Card>
        </Col>
        <Col xs={24} md={12}>
          <Card size="small" title="数据导出" className="ind-card">
            <Flex wrap align="center" gap={8}>
              <Select value={exportStart || undefined} onChange={setExportStart} options={dateOpts} style={{ width: 130 }} size="small" placeholder="起始" />
              <Text type="secondary">至</Text>
              <Select value={exportEnd || undefined} onChange={setExportEnd} options={dateOpts} style={{ width: 130 }} size="small" placeholder="结束" />
              <Button size="small" type="primary" icon={<DownloadOutlined />} onClick={exportCSV}>导出</Button>
            </Flex>
          </Card>
        </Col>
      </Row>

      {showCharts && histData && (
        <>
          <Row gutter={[12, 12]} style={{ marginBottom: 16 }}>
            <Col xs={24} lg={8}>
              <Card size="small" title={`🌡️ ${fmtDate(selectedDate)} 温湿度`} className="ind-card">
                <div className="chart-container">
                  <Line data={toChartData(histData.time, [{ data: histData.temp, label: '温度(°C)' }, { data: histData.hum, label: '湿度(%)' }])} {...lineConfig(['#ef4444', '#0ea5e9'])} />
                </div>
              </Card>
            </Col>
            <Col xs={24} lg={8}>
              <Card size="small" title={`☀️ ${fmtDate(selectedDate)} 光照·土壤`} className="ind-card">
                <div className="chart-container">
                  <Line data={toChartData(histData.time, [{ data: histData.lux, label: '光照(lx)' }, { data: histData.soil, label: '土壤(%)' }])} {...lineConfig(['#f59e0b', '#22c55e'])} />
                </div>
              </Card>
            </Col>
            <Col xs={24} lg={8}>
              <Card size="small" title={`☁️ ${fmtDate(selectedDate)} 空气质量`} className="ind-card">
                <div className="chart-container">
                  <Line data={toChartData(histData.time, [{ data: histData.eco2, label: 'eCO2(ppm)' }, { data: histData.tvoc, label: 'TVOC(ppb)' }])} {...lineConfig(['#06b6d4', '#8b5cf6'])} />
                </div>
              </Card>
            </Col>
          </Row>
          <Card size="small" title="📊 当日统计摘要" className="ind-card" style={{ marginBottom: 16 }}>
            <Table className="ind-table" dataSource={statsData as any[]} columns={statsColumns} size="small" pagination={false} scroll={{ x: 400 }} />
          </Card>
        </>
      )}

      {showLog && (
        <Card size="small" title={`${fmtDate(selectedDate)} 操作日志`}>
          <List
            size="small"
            bordered
            dataSource={logEntries.length > 0 ? logEntries : ['当日无记录']}
            style={{ maxHeight: 400, overflow: 'auto' }}
            renderItem={(item) => (
              <List.Item style={{ padding: '6px 12px', fontFamily: 'monospace', fontSize: 12 }}>{item}</List.Item>
            )}
          />
        </Card>
      )}
    </>
  );
}