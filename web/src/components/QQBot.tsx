import { useState, useEffect, useCallback } from 'react';
import { Card, Row, Col, Input, Switch, Button, Badge, Tag, Typography, Flex, App } from 'antd';
import { SaveOutlined, SendOutlined } from '@ant-design/icons';

const { Title, Text } = Typography;

interface QQBotState {
  appId: string;
  appSecret: string;
  userOpenId: string;
  enabled: boolean;
  tokenValid: boolean;
  gatewayConnected: boolean;
  gatewayReady: boolean;
}

const COMMANDS = [
  { cmd: '状态', desc: '查看环境数据' },
  { cmd: '告警', desc: '查看当前告警' },
  { cmd: '帮助', desc: '显示指令列表' },
];

export default function QQBot() {
  const { message: msg } = App.useApp();
  const [cfg, setCfg] = useState<QQBotState>({
    appId: '', appSecret: '', userOpenId: '', enabled: false,
    tokenValid: false, gatewayConnected: false, gatewayReady: false,
  });
  const [statusText, setStatusText] = useState('未配置');
  const [statusColor, setStatusColor] = useState<'default' | 'success' | 'processing' | 'warning' | 'error'>('default');
  const [gwText, setGwText] = useState('未启用');
  const [gwColor, setGwColor] = useState('#d9d9d9');

  const loadConfig = useCallback(async () => {
    try {
      const res = await fetch('/api/qqbot/config');
      const d = await res.json();
      setCfg((prev) => ({
        ...prev,
        appId: d.appId || '',
        userOpenId: d.userOpenId || '',
        enabled: d.enabled || false,
        tokenValid: d.tokenValid || false,
        gatewayConnected: d.gatewayConnected || false,
        gatewayReady: d.gatewayReady || false,
      }));

      if (!d.appId) { setStatusText('未配置'); setStatusColor('default'); }
      else if (d.gatewayReady) { setStatusText('Gateway 就绪'); setStatusColor('success'); }
      else if (d.tokenValid) { setStatusText('Token 有效'); setStatusColor('processing'); }
      else if (d.enabled) { setStatusText('已启用'); setStatusColor('processing'); }
      else { setStatusText('已禁用'); setStatusColor('warning'); }

      if (d.gatewayReady) { setGwText('已连接'); setGwColor('#52c41a'); }
      else if (d.gatewayConnected) { setGwText('连接中...'); setGwColor('#faad14'); }
      else if (d.enabled) { setGwText('未连接'); setGwColor('#ff4d4f'); }
      else { setGwText('未启用'); setGwColor('#d9d9d9'); }
    } catch { /* ignore */ }
  }, []);

  useEffect(() => {
    loadConfig();
    const timer = setInterval(loadConfig, 15000);
    return () => clearInterval(timer);
  }, [loadConfig]);

  const saveConfig = useCallback(async (overrides?: Partial<QQBotState>) => {
    const merged = { ...cfg, ...overrides };
    const body: Record<string, unknown> = {
      appId: merged.appId,
      userOpenId: merged.userOpenId,
      enabled: merged.enabled,
    };
    if (merged.appSecret) body.appSecret = merged.appSecret;
    try {
      const res = await fetch('/api/qqbot/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
      });
      const d = await res.json();
      if (d.ok) { msg.success('配置已保存'); loadConfig(); }
    } catch (e) { msg.error('保存失败: ' + (e as Error).message); }
  }, [cfg, loadConfig, msg]);

  const testBot = async () => {
    try {
      const res = await fetch('/api/qqbot/test');
      const d = await res.json();
      if (d.ok) msg.success('测试消息发送成功');
      else msg.error('失败: ' + (d.error || '未知错误'));
    } catch (e) { msg.error('请求失败: ' + (e as Error).message); }
  };

  return (
    <>
      <div className="page-header">
        <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>🤖 QQ 机器人</Title>
      </div>

      <Card style={{ marginBottom: 20, boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
        <Flex justify="space-between" align="start" style={{ marginBottom: 16 }}>
          <Flex align="center" gap={10}>
            <span style={{ fontSize: 18 }}>🤖</span>
            <div>
              <Text strong style={{ fontSize: 14 }}>机器人配置</Text>
              <br />
              <Badge status={statusColor} text={statusText} />
            </div>
          </Flex>
          <Switch
            checked={cfg.enabled}
            onChange={(v) => { setCfg({ ...cfg, enabled: v }); saveConfig({ enabled: v }); }}
            checkedChildren="启用"
            unCheckedChildren="禁用"
          />
        </Flex>

        <Row gutter={[16, 16]} style={{ marginBottom: 16 }}>
          <Col xs={24} md={8}>
            <Text type="secondary" style={{ fontSize: 12, display: 'block', marginBottom: 4 }}>AppID</Text>
            <Input
              placeholder="QQ Bot AppID"
              value={cfg.appId}
              onChange={(e) => setCfg({ ...cfg, appId: e.target.value })}
            />
          </Col>
          <Col xs={24} md={8}>
            <Text type="secondary" style={{ fontSize: 12, display: 'block', marginBottom: 4 }}>AppSecret</Text>
            <Input.Password
              placeholder="不修改则留空"
              value={cfg.appSecret}
              onChange={(e) => setCfg({ ...cfg, appSecret: e.target.value })}
            />
          </Col>
          <Col xs={24} md={8}>
            <Text type="secondary" style={{ fontSize: 12, display: 'block', marginBottom: 4 }}>用户 OpenID（可选）</Text>
            <Input
              placeholder="主动推送用，可自动获取"
              value={cfg.userOpenId}
              onChange={(e) => setCfg({ ...cfg, userOpenId: e.target.value })}
            />
          </Col>
        </Row>

        <Flex gap={12}>
          <Button type="primary" icon={<SaveOutlined />} onClick={() => saveConfig()}>保存配置</Button>
          <Button icon={<SendOutlined />} onClick={testBot}>发送测试</Button>
        </Flex>
      </Card>

      <Row gutter={[16, 16]}>
        <Col xs={24} md={12}>
          <Card size="small" title="🛠️ Gateway 状态" className="ind-card" style={{ boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
            <Flex align="center" gap={12} style={{ marginBottom: 12 }}>
              <Badge color={gwColor} />
              <Text strong>{gwText}</Text>
            </Flex>
            <Text type="secondary" style={{ fontSize: 12 }}>
              Gateway 连接后，用户可私聊机器人查询大棚状态
            </Text>
          </Card>
        </Col>
        <Col xs={24} md={12}>
          <Card size="small" title="📝 支持的指令" className="ind-card" style={{ boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
            <Flex vertical gap={8}>
              {COMMANDS.map((c) => (
                <Flex key={c.cmd} align="center" gap={8}>
                  <Tag color="blue" style={{ margin: 0, fontFamily: 'monospace' }}>{c.cmd}</Tag>
                  <Text type="secondary">{c.desc}</Text>
                </Flex>
              ))}
            </Flex>
          </Card>
        </Col>
      </Row>
    </>
  );
}
