import { useState, useEffect } from 'react';
import { Card, Row, Col, Input, Button, Badge, Progress, Tag, Typography, Flex, Descriptions, App } from 'antd';
import { SaveOutlined, WifiOutlined, ClockCircleOutlined } from '@ant-design/icons';
import { useApp } from '../context/AppContext';

const { Title, Text, Link: ALink } = Typography;

function fmtUptime(s: number): string {
  const d = Math.floor(s / 86400);
  const h = Math.floor((s % 86400) / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  if (d > 0) return `${d}天 ${h}时 ${m}分`;
  if (h > 0) return `${h}时 ${m}分 ${sec}秒`;
  return `${m}分 ${sec}秒`;
}

function rssiTag(rssi: number) {
  if (rssi >= -50) return <Tag color="success">极好</Tag>;
  if (rssi >= -70) return <Tag color="processing">良好</Tag>;
  if (rssi >= -80) return <Tag color="warning">一般</Tag>;
  return <Tag color="error">较差</Tag>;
}

const SENSOR_ITEMS = [
  { id: 'sht40' as const, label: 'SHT40 温湿度' },
  { id: 'bh1750' as const, label: 'BH1750 光照' },
  { id: 'sgp30' as const, label: 'SGP30 空气' },
  { id: 'soil' as const, label: 'ADC 土壤' },
];

export default function System() {
  const { message: msg } = App.useApp();
  const { state } = useApp();
  const sys = state.system;
  const sensors = state.sensors;

  const [aiConfigured, setAiConfigured] = useState(false);
  const [apiKey, setApiKey] = useState('');

  useEffect(() => {
    fetch('/api/ai/config')
      .then((r) => r.json())
      .then((d) => setAiConfigured(d.configured ?? false))
      .catch(() => {});
  }, []);

  const saveAIConfig = async () => {
    if (!apiKey) { msg.warning('请输入 API Key'); return; }
    try {
      await fetch('/api/ai/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ apiKey }),
      });
      msg.success('AI API Key 已保存');
      setAiConfigured(true);
      setApiKey('');
    } catch (e) { msg.error('保存失败: ' + (e as Error).message); }
  };

  const heapPct = sys && sys.heap_total && sys.heap_free
    ? +((sys.heap_total - sys.heap_free) / sys.heap_total * 100).toFixed(1)
    : null;

  return (
    <>
      <div className="page-header">
        <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>⚙️ 系统信息</Title>
      </div>

      {/* Sensor status */}
      <Card size="small" title="📡 传感器状态" className="ind-card" style={{ marginBottom: 16, boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
        <Row gutter={[12, 12]}>
          {SENSOR_ITEMS.map((s) => (
            <Col xs={12} md={6} key={s.id}>
              <Flex align="center" gap={8}>
                <span style={{ fontSize: 14 }}>{sensors[s.id] ? '✅' : '❌'}</span>
                <Text style={{ fontSize: 13 }}>{s.label}</Text>
              </Flex>
            </Col>
          ))}
        </Row>
      </Card>

      {/* System info */}
      <Row gutter={[16, 16]} style={{ marginBottom: 16 }}>
        <Col xs={24} lg={12}>
          <Card size="small" title={<><ClockCircleOutlined /> 💻 基本信息</>} className="ind-card" style={{ boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
            <Descriptions column={1} size="small" labelStyle={{ width: 90 }}>
              <Descriptions.Item label="系统时间">{state.time || '--'}</Descriptions.Item>
              <Descriptions.Item label="运行时间">{sys?.uptime != null ? fmtUptime(sys.uptime) : '--'}</Descriptions.Item>
              <Descriptions.Item label="SDK 版本">{sys?.sdk_ver || '--'}</Descriptions.Item>
              <Descriptions.Item label="芯片信息">
                {sys ? `Rev ${sys.chip_rev} · ${sys.cpu_cores}核 · ${sys.cpu_freq}MHz` : '--'}
              </Descriptions.Item>
            </Descriptions>
          </Card>
        </Col>
        <Col xs={24} lg={12}>
          <Card size="small" title={<><WifiOutlined /> 🌐 网络 & 内存</>} className="ind-card" style={{ boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
            <Descriptions column={1} size="small" labelStyle={{ width: 90 }}>
              <Descriptions.Item label="IP 地址">{sys?.ip || '--'}</Descriptions.Item>
              <Descriptions.Item label="MAC">{sys?.mac || '--'}</Descriptions.Item>
              <Descriptions.Item label="WiFi 信号">
                {sys?.rssi != null ? <>{sys.rssi} dBm {rssiTag(sys.rssi)}</> : '--'}
              </Descriptions.Item>
              <Descriptions.Item label="内存使用">
                {heapPct != null ? (
                  <Progress percent={heapPct} size="small" status={heapPct > 85 ? 'exception' : 'normal'} style={{ maxWidth: 200 }} />
                ) : '--'}
              </Descriptions.Item>
            </Descriptions>
          </Card>
        </Col>
      </Row>

      {/* AI config */}
      <Card style={{ marginBottom: 16, boxShadow: '0 2px 8px rgba(0,0,0,0.04)', borderLeft: '3px solid #475569' }}>
        <Flex align="center" gap={10} style={{ marginBottom: 16 }}>
          <span style={{ fontSize: 18 }}>🧠</span>
          <div>
            <Text strong style={{ fontSize: 14 }}>AI 大模型配置</Text>
            <br />
            <Badge status={aiConfigured ? 'success' : 'default'} text={aiConfigured ? '已配置' : '未配置'} />
          </div>
        </Flex>
        <Flex wrap gap={8} align="center">
          <Input.Password
            placeholder="MiniMax API Key"
            value={apiKey}
            onChange={(e) => setApiKey(e.target.value)}
            style={{ maxWidth: 340, flex: 1 }}
          />
          <Button type="primary" icon={<SaveOutlined />} onClick={saveAIConfig}>保存</Button>
        </Flex>
        <Text type="secondary" style={{ fontSize: 12, marginTop: 8, display: 'block' }}>
          使用 MiniMax-M2.7 模型，请在{' '}
          <ALink href="https://platform.minimaxi.com" target="_blank">platform.minimaxi.com</ALink>{' '}
          获取 API Key
        </Text>
      </Card>
    </>
  );
}
