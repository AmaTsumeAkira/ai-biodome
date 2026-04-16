import { useState, useEffect, useCallback } from 'react';

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
  const [cfg, setCfg] = useState<QQBotState>({
    appId: '', appSecret: '', userOpenId: '', enabled: false,
    tokenValid: false, gatewayConnected: false, gatewayReady: false,
  });
  const [statusText, setStatusText] = useState('未配置');
  const [statusColor, setStatusColor] = useState('#9ca3af');
  const [gwText, setGwText] = useState('未启用');
  const [gwColor, setGwColor] = useState('#9ca3af');

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

      if (!d.appId) { setStatusText('未配置'); setStatusColor('#9ca3af'); }
      else if (d.gatewayReady) { setStatusText('Gateway 就绪'); setStatusColor('#16a34a'); }
      else if (d.tokenValid) { setStatusText('Token 有效'); setStatusColor('#2563eb'); }
      else if (d.enabled) { setStatusText('已启用'); setStatusColor('#2563eb'); }
      else { setStatusText('已禁用'); setStatusColor('#ea580c'); }

      if (d.gatewayReady) { setGwText('已连接'); setGwColor('#22c55e'); }
      else if (d.gatewayConnected) { setGwText('连接中...'); setGwColor('#f59e0b'); }
      else if (d.enabled) { setGwText('未连接'); setGwColor('#ef4444'); }
      else { setGwText('未启用'); setGwColor('#9ca3af'); }
    } catch { /* ignore */ }
  }, []);

  useEffect(() => {
    loadConfig();
    const timer = window.setInterval(loadConfig, 15000);
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
      if (d.ok) { alert('配置已保存'); loadConfig(); }
    } catch (e) { alert('保存失败: ' + (e as Error).message); }
  }, [cfg, loadConfig]);

  const testBot = async () => {
    try {
      const res = await fetch('/api/qqbot/test');
      const d = await res.json();
      alert(d.ok ? '测试消息发送成功！' : '失败: ' + (d.error || '未知错误'));
    } catch (e) { alert('请求失败: ' + (e as Error).message); }
  };

  return (
    <div>
      <h1 className="text-xl font-bold text-gray-800 mb-5">QQ 机器人</h1>

      <div className="card mb-6">
        <div className="flex items-center justify-between mb-4">
          <div className="flex items-center gap-3">
            <span className="text-2xl">🤖</span>
            <div>
              <h3 className="font-bold text-gray-800">机器人配置</h3>
              <span className="text-xs font-medium" style={{ color: statusColor }}>{statusText}</span>
            </div>
          </div>
          <label className="relative inline-flex items-center cursor-pointer">
            <input
              type="checkbox"
              className="sr-only peer"
              checked={cfg.enabled}
              onChange={(e) => { const v = e.target.checked; setCfg({ ...cfg, enabled: v }); saveConfig({ enabled: v }); }}
            />
            <div className="w-12 h-6 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-6 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all after:shadow" />
          </label>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
          <div>
            <label className="block text-xs font-semibold text-gray-500 mb-1">AppID</label>
            <input
              type="text"
              placeholder="QQ Bot AppID"
              className="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none"
              value={cfg.appId}
              onChange={(e) => setCfg({ ...cfg, appId: e.target.value })}
            />
          </div>
          <div>
            <label className="block text-xs font-semibold text-gray-500 mb-1">AppSecret</label>
            <input
              type="password"
              placeholder="AppSecret"
              className="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none"
              value={cfg.appSecret}
              onChange={(e) => setCfg({ ...cfg, appSecret: e.target.value })}
            />
          </div>
          <div>
            <label className="block text-xs font-semibold text-gray-500 mb-1">用户 OpenID（可选）</label>
            <input
              type="text"
              placeholder="主动推送用"
              className="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none"
              value={cfg.userOpenId}
              onChange={(e) => setCfg({ ...cfg, userOpenId: e.target.value })}
            />
          </div>
        </div>

        <div className="flex flex-wrap items-center gap-3">
          <button onClick={() => saveConfig()} className="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">保存配置</button>
          <button onClick={testBot} className="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg text-sm font-medium transition-colors">发送测试</button>
        </div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        <div className="card">
          <h3 className="text-sm font-bold text-gray-600 mb-3">Gateway 状态</h3>
          <div className="flex items-center gap-3 mb-3">
            <span className="w-3 h-3 rounded-full" style={{ background: gwColor }} />
            <span className="text-sm font-medium">{gwText}</span>
          </div>
          <p className="text-xs text-gray-400">Gateway 连接后，用户可私聊机器人查询大棚状态</p>
        </div>
        <div className="card">
          <h3 className="text-sm font-bold text-gray-600 mb-3">支持的指令</h3>
          <div className="space-y-2">
            {COMMANDS.map((c) => (
              <div key={c.cmd} className="flex items-center gap-2 text-sm">
                <code className="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">{c.cmd}</code>
                <span className="text-gray-600">{c.desc}</span>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}
