import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import { ConfigProvider, App as AntApp } from 'antd';
import zhCN from 'antd/locale/zh_CN';
import dayjs from 'dayjs';
import 'dayjs/locale/zh-cn';
import { AppProvider } from './context/AppContext';
import App from './App';
import './index.css';

dayjs.locale('zh-cn');

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <ConfigProvider
      locale={zhCN}
      theme={{
        token: {
          colorPrimary: '#475569',
          colorWarning: '#f59e0b',
          colorError: '#ef4444',
          colorSuccess: '#22c55e',
          colorInfo: '#0ea5e9',
          borderRadius: 4,
          fontFamily: "'PingFang SC', 'Microsoft YaHei', -apple-system, sans-serif",
          colorBgContainer: '#ffffff',
          colorBgLayout: '#f1f5f9',
          colorBorderSecondary: '#e2e8f0',
          fontSize: 13,
          boxShadow: '0 2px 8px rgba(0, 0, 0, 0.06)',
          boxShadowSecondary: '0 4px 16px rgba(0, 0, 0, 0.08)',
        },
        components: {
          Card: { borderRadiusLG: 4, paddingLG: 16, boxShadowTertiary: '0 1px 4px rgba(0,0,0,0.06)' },
          Menu: { darkItemBg: 'transparent', darkSubMenuItemBg: 'transparent', darkItemSelectedBg: 'rgba(245,158,11,0.15)', darkItemSelectedColor: '#f59e0b' },
          Table: { headerBg: '#f8fafc', borderColor: '#e2e8f0' },
          Tag: { borderRadiusSM: 2 },
          Button: { borderRadius: 4, primaryShadow: '0 2px 6px rgba(71,85,105,0.3)' },
          Switch: { colorPrimary: '#22c55e' },
          Progress: { colorSuccess: '#22c55e' },
        },
      }}
    >
      <AntApp>
        <AppProvider>
          <App />
        </AppProvider>
      </AntApp>
    </ConfigProvider>
  </StrictMode>,
);
