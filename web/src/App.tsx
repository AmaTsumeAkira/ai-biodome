import { useState, useCallback, useEffect } from 'react';
import { Layout, Menu, Drawer, Grid, Button, Typography, Flex } from 'antd';
import {
  DashboardOutlined,
  DeploymentUnitOutlined,
  ControlOutlined,
  HistoryOutlined,
  RobotOutlined,
  SettingOutlined,
  MenuOutlined,
} from '@ant-design/icons';
import { useApp } from './context/AppContext';
import Dashboard from './components/Dashboard';
import Control from './components/Control';
import History from './components/History';
import QQBot from './components/QQBot';
import System from './components/System';
import DigitalTwin from './components/DigitalTwin';
import AIChat from './components/AIChat';
import type { Tab } from './types';

const { Sider, Content, Header } = Layout;
const { useBreakpoint } = Grid;
const { Text } = Typography;

const menuItems = [
  { key: 'dashboard', icon: <DashboardOutlined />, label: '仪表盘' },
  { key: 'twin', icon: <DeploymentUnitOutlined />, label: '数字孪生' },
  { key: 'control', icon: <ControlOutlined />, label: '设备控制' },
  { key: 'history', icon: <HistoryOutlined />, label: '历史数据' },
  { key: 'qqbot', icon: <RobotOutlined />, label: 'QQ 机器人' },
  { key: 'system', icon: <SettingOutlined />, label: '系统信息' },
];

export default function App() {
  const { state } = useApp();
  const [currentTab, setCurrentTab] = useState<Tab>('dashboard');
  const [drawerOpen, setDrawerOpen] = useState(false);
  const [gwStatus, setGwStatus] = useState<string>('未启用');
  const [gwColor, setGwColor] = useState('#d9d9d9');
  const screens = useBreakpoint();
  const isMobile = !screens.md;

  useEffect(() => {
    const poll = () => {
      fetch('/api/qqbot/config')
        .then((r) => r.json())
        .then((d) => {
          if (d.gatewayReady) { setGwStatus('GW 在线'); setGwColor('#52c41a'); }
          else if (d.gatewayConnected) { setGwStatus('GW 连接中'); setGwColor('#faad14'); }
          else if (d.enabled) { setGwStatus('GW 离线'); setGwColor('#ff4d4f'); }
          else { setGwStatus('GW 未启用'); setGwColor('#d9d9d9'); }
        })
        .catch(() => {});
    };
    const t = setTimeout(poll, 3000);
    const i = setInterval(poll, 15000);
    return () => { clearTimeout(t); clearInterval(i); };
  }, []);

  const switchTab = useCallback((tab: Tab) => {
    setCurrentTab(tab);
    setDrawerOpen(false);
  }, []);

  const wsColor = state.wsStatus === 'connected' ? '#52c41a' : state.wsStatus === 'connecting' ? '#faad14' : '#ff4d4f';
  const wsText = state.wsStatus === 'connected' ? '已连接' : state.wsStatus === 'connecting' ? '连接中' : '未连接';

  const renderContent = () => {
    switch (currentTab) {
      case 'dashboard': return <Dashboard />;
      case 'twin': return <DigitalTwin />;
      case 'control': return <Control />;
      case 'history': return <History />;
      case 'qqbot': return <QQBot />;
      case 'system': return <System />;
    }
  };

  const sidebarContent = (
    <Menu
      theme="dark"
      mode="inline"
      selectedKeys={[currentTab]}
      items={menuItems}
      onClick={({ key }) => switchTab(key as Tab)}
      style={{ borderInlineEnd: 'none' }}
    />
  );

  const statusFooter = (
    <div style={{ padding: '10px 16px', borderTop: '1px solid #334155', fontSize: 12 }}>
      <Flex gap={6} align="center" style={{ marginBottom: 4 }}>
        <span className="ind-led" style={{ color: wsColor }} />
        <Text style={{ color: '#94a3b8', fontSize: 11 }}>WS: {wsText}</Text>
      </Flex>
      <Flex gap={6} align="center">
        <span className="ind-led" style={{ color: gwColor }} />
        <Text style={{ color: '#94a3b8', fontSize: 11 }}>{gwStatus}</Text>
      </Flex>
    </div>
  );

  if (isMobile) {
    return (
      <Layout style={{ minHeight: '100vh' }}>
        <Header
          style={{
            position: 'fixed', top: 0, left: 0, right: 0, zIndex: 100,
            background: '#1e293b', height: 48, padding: '0 12px',
            display: 'flex', alignItems: 'center', justifyContent: 'space-between',
            borderBottom: '1px solid #334155', lineHeight: '48px',
          }}
        >
          <Flex align="center" gap={10}>
            <Button type="text" icon={<MenuOutlined style={{ color: '#94a3b8' }} />} onClick={() => setDrawerOpen(true)} />
            <Text strong style={{ fontSize: 14, color: '#f1f5f9' }}>AI-Biodome</Text>
          </Flex>
          <Flex gap={8} align="center">
            <span className="ind-led" style={{ color: wsColor }} />
            <Text style={{ fontSize: 11, color: '#94a3b8' }}>{wsText}</Text>
          </Flex>
        </Header>

        <Drawer
          open={drawerOpen}
          onClose={() => setDrawerOpen(false)}
          placement="left"
          width={260}
          styles={{ body: { padding: 0, background: '#1e293b', display: 'flex', flexDirection: 'column' }, header: { display: 'none' } }}
        >
          <div style={{ padding: '14px 16px', borderBottom: '1px solid #334155' }}>
            <Text style={{ color: '#f1f5f9', fontSize: 15, fontWeight: 700 }}>AI-Biodome</Text>
            <br />
            <Text style={{ color: '#64748b', fontSize: 11, letterSpacing: 0.5 }}>GREENHOUSE CONTROL SYSTEM</Text>
          </div>
          {sidebarContent}
          <div style={{ marginTop: 'auto' }}>{statusFooter}</div>
        </Drawer>

        <Content style={{ marginTop: 48, padding: 12, paddingBottom: 68, background: '#f1f5f9', minHeight: 'calc(100vh - 48px)' }}>
          {renderContent()}
        </Content>

        <div className="bottom-tabs">
          {menuItems.map((item) => (
            <div
              key={item.key}
              className={`bottom-tab ${currentTab === item.key ? 'active' : ''}`}
              onClick={() => switchTab(item.key as Tab)}
            >
              {item.icon}
              <span>{item.label}</span>
            </div>
          ))}
        </div>

        <AIChat />
      </Layout>
    );
  }

  return (
    <Layout style={{ minHeight: '100vh' }}>
      <Sider
        width={200}
        theme="dark"
        style={{ position: 'fixed', left: 0, top: 0, bottom: 0, zIndex: 10, display: 'flex', flexDirection: 'column', background: 'linear-gradient(180deg, #1e293b 0%, #0f172a 100%)' }}
      >
        <div style={{ padding: '16px 16px 12px', borderBottom: '1px solid #334155', background: 'rgba(255,255,255,0.03)' }}>
          <Text style={{ color: '#f1f5f9', fontSize: 16, fontWeight: 800, display: 'block', letterSpacing: 0.5 }}>AI-Biodome</Text>
          <Text style={{ color: '#64748b', fontSize: 10, letterSpacing: 1 }}>GREENHOUSE CONTROL SYSTEM</Text>
        </div>
        {sidebarContent}
        <div style={{ marginTop: 'auto' }}>{statusFooter}</div>
      </Sider>
      <Layout style={{ marginLeft: 200 }}>
        <Content style={{ padding: 20, background: '#f1f5f9', minHeight: '100vh' }}>
          {renderContent()}
        </Content>
      </Layout>
      <AIChat />
    </Layout>
  );
}
