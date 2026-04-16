import { useState, useCallback, useEffect } from 'react';
import { useApp } from './context/AppContext';
import Dashboard from './components/Dashboard';
import Control from './components/Control';
import History from './components/History';
import QQBot from './components/QQBot';
import System from './components/System';
import AIChat from './components/AIChat';
import type { Tab } from './types';

const tabs: { id: Tab; icon: string; label: string; mLabel: string }[] = [
  { id: 'dashboard', icon: '📊', label: '仪表盘', mLabel: '仪表盘' },
  { id: 'control', icon: '⚙️', label: '设备控制', mLabel: '控制' },
  { id: 'history', icon: '📁', label: '历史数据', mLabel: '历史' },
  { id: 'qqbot', icon: '🤖', label: 'QQ 机器人', mLabel: '机器人' },
  { id: 'system', icon: '📋', label: '系统信息', mLabel: '系统' },
];

export default function App() {
  const { state } = useApp();
  const [currentTab, setCurrentTab] = useState<Tab>('dashboard');
  const [sidebarOpen, setSidebarOpen] = useState(false);
  const [gwStatus, setGwStatus] = useState<'off' | 'pending' | 'on'>('off');
  const [gwLabel, setGwLabel] = useState('Gateway');

  useEffect(() => {
    const poll = () => {
      fetch('/api/qqbot/config').then(r => r.json()).then(d => {
        if (d.gatewayReady) { setGwStatus('on'); setGwLabel('GW 在线'); }
        else if (d.gatewayConnected) { setGwStatus('pending'); setGwLabel('GW 连接中'); }
        else if (d.enabled) { setGwStatus('off'); setGwLabel('GW 离线'); }
        else { setGwStatus('off'); setGwLabel('GW 未启用'); }
      }).catch(() => {});
    };
    const t = window.setTimeout(poll, 3000);
    const i = window.setInterval(poll, 15000);
    return () => { clearTimeout(t); clearInterval(i); };
  }, []);

  const switchTab = useCallback((tab: Tab) => {
    setCurrentTab(tab);
    setSidebarOpen(false);
  }, []);

  const wsDotClass = state.wsStatus === 'connected' ? 'on' : state.wsStatus === 'connecting' ? 'pending' : 'off';
  const wsText = state.wsStatus === 'connected' ? '已连接' : state.wsStatus === 'connecting' ? '连接中' : '未连接';

  return (
    <>
      {/* Sidebar overlay (mobile) */}
      <div
        className={`sidebar-overlay ${sidebarOpen ? 'show' : ''}`}
        onClick={() => setSidebarOpen(false)}
      />

      {/* Sidebar */}
      <nav className={`sidebar ${sidebarOpen ? 'open' : ''}`}>
        <div className="sidebar-brand">
          <div className="logo">🌱</div>
          <div>
            <div className="brand-text">AI-Biodome</div>
            <div className="brand-sub">智慧大棚控制台</div>
          </div>
        </div>
        <div className="nav-list">
          {tabs.map((t) => (
            <button
              key={t.id}
              className={`nav-item ${currentTab === t.id ? 'active' : ''}`}
              onClick={() => switchTab(t.id)}
            >
              <span className="icon">{t.icon}</span>
              <span className="nav-label">{t.label}</span>
            </button>
          ))}
        </div>
        <div className="sidebar-footer">
          <div className="conn-badge">
            <span className={`conn-dot ${wsDotClass}`} />
            <span>{wsText}</span>
          </div>
          <div className="conn-badge" style={{ marginTop: 4 }}>
            <span className={`conn-dot ${gwStatus}`} />
            <span>{gwLabel}</span>
          </div>
        </div>
      </nav>

      {/* Top bar (mobile) */}
      <div className="topbar">
        <button className="menu-btn" onClick={() => setSidebarOpen(true)}>☰</button>
        <span className="font-bold text-sm text-gray-700">🌱 AI-Biodome</span>
        <span className={`conn-dot ${wsDotClass}`} style={{ width: 8, height: 8, borderRadius: '50%' }} />
      </div>

      {/* Bottom tabs (mobile) */}
      <div className="bottom-tabs">
        {tabs.map((t) => (
          <button
            key={t.id}
            className={`tab-btn ${currentTab === t.id ? 'active' : ''}`}
            onClick={() => switchTab(t.id)}
          >
            <span className="t-icon">{t.icon}</span>{t.mLabel}
          </button>
        ))}
      </div>

      {/* AI Chat */}
      <AIChat />

      {/* Main content */}
      <div className="main-wrap">
        {currentTab === 'dashboard' && <Dashboard />}
        {currentTab === 'control' && <Control />}
        {currentTab === 'history' && <History />}
        {currentTab === 'qqbot' && <QQBot />}
        {currentTab === 'system' && <System />}
      </div>
    </>
  );
}
