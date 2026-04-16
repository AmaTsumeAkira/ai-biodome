import { useApp } from '../context/AppContext';
import { useCallback } from 'react';

const DEVICES = [
  { id: 'pump', icon: '💧', label: '水泵', desc: '土壤灌溉', bgCls: 'bg-blue-50' },
  { id: 'light', icon: '💡', label: '补光灯', desc: '植物补光', bgCls: 'bg-yellow-50' },
  { id: 'heater', icon: '🔥', label: '加热垫', desc: '温度调节', bgCls: 'bg-red-50' },
  { id: 'fan', icon: '🌀', label: '排风扇', desc: '通风换气', bgCls: 'bg-cyan-50' },
] as const;

export default function Control() {
  const { state, wsSend } = useApp();
  const { state: devState, sched } = state;
  const isAuto = devState.mode === 'auto';

  const toggleMode = () => {
    wsSend({ action: 'set_mode', mode: isAuto ? 'manual' : 'auto' });
  };

  const toggleDevice = (dev: string) => {
    if (isAuto) return;
    const cur = devState[dev as keyof typeof devState] as number;
    wsSend({ action: 'set_device', device: dev, state: cur ? 0 : 1 });
  };

  const sendSchedule = useCallback((patch: Partial<typeof sched>) => {
    const merged = { ...sched, ...patch };
    wsSend({
      action: 'set_sched',
      fan_en: merged.fan_en,
      fan_start: merged.fan_start || '00:00',
      fan_end: merged.fan_end || '00:00',
      light_en: merged.light_en,
      light_start: merged.light_start || '00:00',
      light_end: merged.light_end || '00:00',
    });
  }, [sched, wsSend]);

  return (
    <div>
      <h1 className="text-xl font-bold text-gray-800 mb-5">设备控制</h1>

      {/* Mode toggle */}
      <div className="card mb-6">
        <div className="flex items-center justify-between">
          <div>
            <h3 className="font-bold text-gray-800">运行模式</h3>
            <p className="text-xs text-gray-400 mt-1">自动模式下设备根据传感器数据自动调节</p>
          </div>
          <div className="flex items-center gap-3">
            <span className="text-sm font-bold" style={{ color: isAuto ? '#16a34a' : '#ea580c' }}>
              {isAuto ? '自动' : '手动'}
            </span>
            <label className="relative inline-flex items-center cursor-pointer">
              <input type="checkbox" className="sr-only peer" checked={isAuto} onChange={toggleMode} />
              <div className="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:bg-green-500 peer-checked:after:translate-x-6 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all after:shadow" />
            </label>
          </div>
        </div>
      </div>

      {/* Device cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-6">
        {DEVICES.map((d) => {
          const isOn = (devState[d.id] as number) === 1;
          return (
            <div key={d.id} className="card dev-card">
              <div className="dev-info">
                <div className={`dev-icon ${d.bgCls}`}><span>{d.icon}</span></div>
                <div>
                  <div className="font-bold text-gray-800 text-sm">{d.label}</div>
                  <div className="text-xs text-gray-400">{d.desc}</div>
                </div>
              </div>
              <div
                className={`toggle ${isOn ? 'on' : ''} ${isAuto ? 'disabled' : ''}`}
                onClick={() => toggleDevice(d.id)}
              />
            </div>
          );
        })}
      </div>

      {/* Schedule */}
      <div className="card">
        <h3 className="font-bold text-gray-800 mb-4">⏰ 定时任务</h3>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          <div className="bg-slate-50 rounded-xl p-4">
            <div className="flex items-center justify-between mb-3">
              <span className="font-semibold text-sm text-gray-700">🌀 定时通风</span>
              <label className="relative inline-flex items-center cursor-pointer">
                <input
                  type="checkbox"
                  className="sr-only peer"
                  checked={sched.fan_en}
                  onChange={(e) => sendSchedule({ fan_en: e.target.checked })}
                />
                <div className="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all" />
              </label>
            </div>
            <div className="flex items-center gap-2 text-sm">
              <input
                type="time"
                className="px-2 py-1 rounded border text-sm"
                value={sched.fan_start}
                onChange={(e) => sendSchedule({ fan_start: e.target.value })}
              />
              <span className="text-gray-400">至</span>
              <input
                type="time"
                className="px-2 py-1 rounded border text-sm"
                value={sched.fan_end}
                onChange={(e) => sendSchedule({ fan_end: e.target.value })}
              />
            </div>
          </div>
          <div className="bg-slate-50 rounded-xl p-4">
            <div className="flex items-center justify-between mb-3">
              <span className="font-semibold text-sm text-gray-700">💡 定时补光</span>
              <label className="relative inline-flex items-center cursor-pointer">
                <input
                  type="checkbox"
                  className="sr-only peer"
                  checked={sched.light_en}
                  onChange={(e) => sendSchedule({ light_en: e.target.checked })}
                />
                <div className="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all" />
              </label>
            </div>
            <div className="flex items-center gap-2 text-sm">
              <input
                type="time"
                className="px-2 py-1 rounded border text-sm"
                value={sched.light_start}
                onChange={(e) => sendSchedule({ light_start: e.target.value })}
              />
              <span className="text-gray-400">至</span>
              <input
                type="time"
                className="px-2 py-1 rounded border text-sm"
                value={sched.light_end}
                onChange={(e) => sendSchedule({ light_end: e.target.value })}
              />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
