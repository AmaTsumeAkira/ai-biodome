import { useState, useEffect } from 'react';
import { useApp } from '../context/AppContext';

function fmtUptime(s: number): string {
  const d = Math.floor(s / 86400);
  const h = Math.floor((s % 86400) / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  if (d > 0) return `${d}天 ${h}时 ${m}分`;
  if (h > 0) return `${h}时 ${m}分 ${sec}秒`;
  return `${m}分 ${sec}秒`;
}

function rssiQuality(rssi: number): { text: string; color: string } {
  if (rssi >= -50) return { text: '极好', color: '#16a34a' };
  if (rssi >= -70) return { text: '良好', color: '#2563eb' };
  if (rssi >= -80) return { text: '一般', color: '#d97706' };
  return { text: '较差', color: '#dc2626' };
}

export default function System() {
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
    if (!apiKey) { alert('请输入 API Key'); return; }
    try {
      await fetch('/api/ai/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ apiKey }),
      });
      alert('AI API Key 已保存');
      setAiConfigured(true);
      setApiKey('');
    } catch (e) { alert('保存失败: ' + (e as Error).message); }
  };

  const heap = sys && sys.heap_total && sys.heap_free
    ? ((sys.heap_total - sys.heap_free) / sys.heap_total * 100).toFixed(1)
    : null;

  const rq = sys?.rssi ? rssiQuality(sys.rssi) : null;

  const sensorItems: { id: keyof typeof sensors; label: string }[] = [
    { id: 'sht40', label: 'SHT40 温湿度' },
    { id: 'bh1750', label: 'BH1750 光照' },
    { id: 'sgp30', label: 'SGP30 空气' },
    { id: 'soil', label: 'ADC 土壤' },
  ];

  return (
    <div>
      <h1 className="text-xl font-bold text-gray-800 mb-5">系统信息</h1>

      {/* Sensor status */}
      <div className="card mb-6">
        <h3 className="text-sm font-bold text-gray-600 mb-3">传感器状态</h3>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-3">
          {sensorItems.map((s) => (
            <div key={s.id} className="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
              <span className={`conn-dot ${sensors[s.id] ? 'on' : 'off'}`} />
              <span className="text-sm text-gray-600">{s.label}</span>
            </div>
          ))}
        </div>
      </div>

      {/* System info cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">系统时间</div>
          <div className="text-lg font-bold text-gray-800">{state.time || '--'}</div>
        </div>
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">内存使用</div>
          <div className="text-lg font-bold text-gray-800">{heap != null ? `${heap} %` : '-- %'}</div>
        </div>
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">WiFi 信号</div>
          <div className="text-lg font-bold text-gray-800">
            {sys?.rssi != null ? `${sys.rssi} dBm` : '--'}{' '}
            {rq && <span className="text-sm font-normal" style={{ color: rq.color }}>{rq.text}</span>}
          </div>
        </div>
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">运行时间</div>
          <div className="text-lg font-bold text-gray-800">{sys?.uptime != null ? fmtUptime(sys.uptime) : '--'}</div>
        </div>
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">芯片信息</div>
          <div className="text-sm font-bold text-gray-800">
            {sys ? `Rev ${sys.chip_rev} · ${sys.cpu_cores}核 · ${sys.cpu_freq}MHz` : '--'}
          </div>
        </div>
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">网络</div>
          <div className="text-sm font-bold text-gray-800">{sys?.ip || '--'}</div>
          <div className="text-xs text-gray-400 mt-1">{sys?.mac || '--'}</div>
        </div>
        <div className="card card-sm">
          <div className="text-xs text-gray-400 mb-1">SDK 版本</div>
          <div className="text-sm font-bold text-gray-800">{sys?.sdk_ver || '--'}</div>
        </div>
      </div>

      {/* AI config */}
      <div className="card mt-6">
        <div className="flex items-center gap-3 mb-3">
          <span className="text-xl">🧠</span>
          <div>
            <h3 className="font-bold text-gray-800 text-sm">AI 大模型配置</h3>
            <span className="text-xs" style={{ color: aiConfigured ? '#16a34a' : '#9ca3af' }}>
              {aiConfigured ? '已配置' : '未配置'}
            </span>
          </div>
        </div>
        <div className="flex flex-wrap items-center gap-3">
          <input
            type="password"
            placeholder="MiniMax API Key"
            className="flex-1 min-w-[200px] px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none"
            value={apiKey}
            onChange={(e) => setApiKey(e.target.value)}
          />
          <button onClick={saveAIConfig} className="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">保存</button>
        </div>
        <p className="text-xs text-gray-400 mt-2">
          使用 MiniMax-M2.7 模型，请在{' '}
          <a href="https://platform.minimaxi.com" target="_blank" rel="noopener noreferrer" className="text-blue-500 underline">
            platform.minimaxi.com
          </a>{' '}
          获取 API Key
        </p>
      </div>
    </div>
  );
}
