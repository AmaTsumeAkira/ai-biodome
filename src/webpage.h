#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>AI-Biodome 智慧大棚</title>
<script src="https://cdn.tailwindcss.com"></script>
<script src="https://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js"></script>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{--sidebar-w:220px;--sidebar-collapsed:64px;--primary:#3b82f6;--accent:#06b6d4}
body{font-family:'Segoe UI','PingFang SC','Microsoft YaHei',sans-serif;background:#f1f5f9;color:#334155;overflow-x:hidden}

/* Sidebar */
.sidebar{position:fixed;left:0;top:0;bottom:0;width:var(--sidebar-w);background:linear-gradient(180deg,#0f172a 0%,#1e293b 100%);z-index:50;display:flex;flex-direction:column;transition:transform .3s}
.sidebar-brand{padding:20px 16px;border-bottom:1px solid rgba(255,255,255,0.08);display:flex;align-items:center;gap:12px}
.sidebar-brand .logo{width:36px;height:36px;background:linear-gradient(135deg,#22c55e,#06b6d4);border-radius:10px;display:flex;align-items:center;justify-content:center;font-size:20px;flex-shrink:0}
.sidebar-brand .brand-text{color:#f8fafc;font-size:15px;font-weight:700;white-space:nowrap}
.sidebar-brand .brand-sub{color:#64748b;font-size:11px}
.nav-list{flex:1;padding:12px 8px;display:flex;flex-direction:column;gap:2px}
.nav-item{display:flex;align-items:center;gap:12px;padding:10px 14px;border-radius:10px;color:#94a3b8;font-size:14px;font-weight:500;cursor:pointer;border:none;background:none;width:100%;text-align:left;transition:all .15s;white-space:nowrap}
.nav-item:hover{background:rgba(255,255,255,0.06);color:#e2e8f0}
.nav-item.active{background:rgba(59,130,246,0.15);color:#60a5fa}
.nav-item .icon{font-size:18px;width:24px;text-align:center;flex-shrink:0}
.sidebar-footer{padding:12px 16px;border-top:1px solid rgba(255,255,255,0.08)}
.conn-badge{display:flex;align-items:center;gap:6px;font-size:12px;color:#64748b}
.conn-dot{width:8px;height:8px;border-radius:50%;flex-shrink:0}
.conn-dot.on{background:#22c55e;box-shadow:0 0 6px #22c55e}
.conn-dot.off{background:#ef4444}
.conn-dot.pending{background:#f59e0b;animation:pulse 1.5s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}

/* Main content */
.main-wrap{margin-left:var(--sidebar-w);min-height:100vh;padding:24px;transition:margin .3s}

/* Top bar (mobile) */
.topbar{display:none;position:fixed;top:0;left:0;right:0;height:56px;background:#fff;border-bottom:1px solid #e2e8f0;z-index:40;padding:0 16px;align-items:center;justify-content:space-between}
.topbar .menu-btn{width:36px;height:36px;border:none;background:#f1f5f9;border-radius:8px;font-size:18px;cursor:pointer}

/* Bottom tabs (mobile) */
.bottom-tabs{display:none;position:fixed;bottom:0;left:0;right:0;height:60px;background:#fff;border-top:1px solid #e2e8f0;z-index:40;justify-content:space-around;align-items:center;padding-bottom:env(safe-area-inset-bottom)}
.tab-btn{display:flex;flex-direction:column;align-items:center;gap:2px;border:none;background:none;color:#94a3b8;font-size:10px;cursor:pointer;padding:4px 8px}
.tab-btn.active{color:var(--primary)}
.tab-btn .t-icon{font-size:20px}

/* Panels */
.panel{display:none}
.panel.active{display:block}

/* Cards */
.card{background:#fff;border-radius:16px;box-shadow:0 1px 3px rgba(0,0,0,0.06);padding:20px;border:1px solid #f1f5f9}
.card-sm{padding:16px}

/* Sensor cards */
.sensor-card{display:flex;flex-direction:column;gap:8px;position:relative;overflow:hidden}
.sensor-card::before{content:'';position:absolute;top:0;left:0;right:0;height:3px;border-radius:16px 16px 0 0}
.sensor-card.sc-temp::before{background:linear-gradient(90deg,#ef4444,#f97316)}
.sensor-card.sc-hum::before{background:linear-gradient(90deg,#3b82f6,#06b6d4)}
.sensor-card.sc-lux::before{background:linear-gradient(90deg,#f59e0b,#eab308)}
.sensor-card.sc-soil::before{background:linear-gradient(90deg,#22c55e,#10b981)}
.sensor-card.sc-co2::before{background:linear-gradient(90deg,#14b8a6,#06b6d4)}
.sensor-card.sc-tvoc::before{background:linear-gradient(90deg,#a855f7,#8b5cf6)}
.sensor-icon{font-size:28px}
.sensor-val{font-size:28px;font-weight:800;color:#0f172a;font-variant-numeric:tabular-nums}
.sensor-label{font-size:12px;color:#64748b;font-weight:500}
.s-badge{display:inline-block;padding:2px 8px;border-radius:20px;font-size:11px;font-weight:600}
.s-badge.b-ok{background:#dcfce7;color:#16a34a}
.s-badge.b-warn{background:#fef3c7;color:#d97706}
.s-badge.b-high{background:#fee2e2;color:#dc2626}
.s-badge.b-low{background:#dbeafe;color:#2563eb}

/* Device toggles */
.dev-card{display:flex;align-items:center;justify-content:space-between;padding:16px 20px}
.dev-info{display:flex;align-items:center;gap:12px}
.dev-icon{width:42px;height:42px;border-radius:12px;display:flex;align-items:center;justify-content:center;font-size:22px}
.toggle{position:relative;width:44px;height:24px;border-radius:12px;background:#cbd5e1;cursor:pointer;transition:.2s}
.toggle.on{background:#3b82f6}
.toggle::after{content:'';position:absolute;top:2px;left:2px;width:20px;height:20px;border-radius:50%;background:#fff;transition:.2s;box-shadow:0 1px 3px rgba(0,0,0,.2)}
.toggle.on::after{left:22px}
.toggle.disabled{opacity:.5;cursor:not-allowed}

/* Charts */
.chart-box{height:280px;border-radius:12px;background:#f8fafc;padding:4px}

/* Alerts */
.alert-item{padding:8px 12px;border-radius:8px;font-size:13px;margin-bottom:6px}
.alert-danger{background:#fef2f2;color:#dc2626;border-left:3px solid #ef4444}
.alert-warn{background:#fffbeb;color:#d97706;border-left:3px solid #f59e0b}

/* AI Chat */
.ai-float-btn{position:fixed;bottom:80px;right:20px;width:52px;height:52px;border-radius:50%;background:linear-gradient(135deg,#3b82f6,#8b5cf6);color:#fff;border:none;font-size:24px;cursor:pointer;box-shadow:0 4px 12px rgba(59,130,246,.4);z-index:35;display:flex;align-items:center;justify-content:center;transition:transform .2s}
.ai-float-btn:hover{transform:scale(1.1)}
.ai-drawer{position:fixed;bottom:0;right:0;width:380px;max-width:100vw;height:70vh;max-height:600px;background:#fff;border-radius:16px 16px 0 0;box-shadow:-4px -4px 20px rgba(0,0,0,.12);z-index:55;display:flex;flex-direction:column;transform:translateY(100%);transition:transform .3s;overflow:hidden}
.ai-drawer.open{transform:translateY(0)}
.ai-drawer-head{padding:14px 16px;background:linear-gradient(135deg,#3b82f6,#8b5cf6);color:#fff;display:flex;align-items:center;justify-content:space-between;flex-shrink:0}
.ai-drawer-body{flex:1;overflow-y:auto;padding:12px}
.ai-drawer-foot{padding:10px;border-top:1px solid #e2e8f0;display:flex;gap:8px}
@media(max-width:768px){
  .ai-float-btn{bottom:72px;right:12px;width:46px;height:46px;font-size:20px}
  .ai-drawer{width:100%;height:60vh;border-radius:16px 16px 0 0}
}

/* Responsive */
@media(max-width:768px){
  .sidebar{transform:translateX(-100%)}
  .sidebar.open{transform:translateX(0)}
  .sidebar-overlay{display:none;position:fixed;inset:0;background:rgba(0,0,0,.4);z-index:45}
  .sidebar-overlay.show{display:block}
  .main-wrap{margin-left:0;padding:72px 12px 76px}
  .topbar{display:flex}
  .bottom-tabs{display:flex}
}
@media(min-width:769px) and (max-width:1024px){
  .sidebar{width:var(--sidebar-collapsed)}
  .sidebar .brand-text,.sidebar .brand-sub,.sidebar .nav-label{display:none}
  .sidebar .nav-item{justify-content:center;padding:12px}
  .sidebar .sidebar-footer .conn-badge span:not(.conn-dot){display:none}
  .main-wrap{margin-left:var(--sidebar-collapsed)}
}
</style>
</head>
<body>
<!-- Sidebar Overlay (mobile) -->
<div class="sidebar-overlay" id="sidebar-overlay" onclick="closeSidebar()"></div>

<!-- Sidebar -->
<nav class="sidebar" id="sidebar">
  <div class="sidebar-brand">
    <div class="logo">🌱</div>
    <div>
      <div class="brand-text">AI-Biodome</div>
      <div class="brand-sub">智慧大棚控制台</div>
    </div>
  </div>
  <div class="nav-list">
    <button class="nav-item active" onclick="switchTab('dashboard')" data-tab="dashboard">
      <span class="icon">📊</span><span class="nav-label">仪表盘</span>
    </button>
    <button class="nav-item" onclick="switchTab('control')" data-tab="control">
      <span class="icon">⚙️</span><span class="nav-label">设备控制</span>
    </button>
    <button class="nav-item" onclick="switchTab('history')" data-tab="history">
      <span class="icon">📁</span><span class="nav-label">历史数据</span>
    </button>
    <button class="nav-item" onclick="switchTab('qqbot')" data-tab="qqbot">
      <span class="icon">🤖</span><span class="nav-label">QQ 机器人</span>
    </button>
    <button class="nav-item" onclick="switchTab('system')" data-tab="system">
      <span class="icon">📋</span><span class="nav-label">系统信息</span>
    </button>
  </div>
  <div class="sidebar-footer">
    <div class="conn-badge">
      <span class="conn-dot off" id="ws-dot"></span>
      <span id="ws-status-text">未连接</span>
    </div>
    <div class="conn-badge" style="margin-top:4px">
      <span class="conn-dot off" id="gw-dot-side"></span>
      <span id="gw-status-side">Gateway</span>
    </div>
  </div>
</nav>

<!-- Top Bar (mobile) -->
<div class="topbar" id="topbar">
  <button class="menu-btn" onclick="openSidebar()">☰</button>
  <span class="font-bold text-sm text-gray-700">🌱 AI-Biodome</span>
  <div class="flex items-center gap-2">
    <span class="conn-dot off" id="ws-dot-mobile" style="width:8px;height:8px;border-radius:50%"></span>
  </div>
</div>

<!-- Bottom Tabs (mobile) -->
<div class="bottom-tabs" id="bottom-tabs">
  <button class="tab-btn active" onclick="switchTab('dashboard')" data-tab="dashboard"><span class="t-icon">📊</span>仪表盘</button>
  <button class="tab-btn" onclick="switchTab('control')" data-tab="control"><span class="t-icon">⚙️</span>控制</button>
  <button class="tab-btn" onclick="switchTab('history')" data-tab="history"><span class="t-icon">📁</span>历史</button>
  <button class="tab-btn" onclick="switchTab('qqbot')" data-tab="qqbot"><span class="t-icon">🤖</span>机器人</button>
  <button class="tab-btn" onclick="switchTab('system')" data-tab="system"><span class="t-icon">📋</span>系统</button>
</div>

<!-- AI Chat Float Button -->
<button class="ai-float-btn" id="ai-float-btn" onclick="toggleAIChat()">💬</button>

<!-- AI Chat Drawer -->
<div class="ai-drawer" id="ai-drawer">
  <div class="ai-drawer-head">
    <div class="flex items-center gap-2"><span class="text-lg">🧠</span><span class="font-bold text-sm">AI 智能助手</span></div>
    <div class="flex items-center gap-2">
      <button id="btn-ai-analyze" onclick="askAI()" class="px-3 py-1 bg-white/20 hover:bg-white/30 rounded-lg text-xs font-medium transition-colors">环境分析</button>
      <button onclick="toggleAIChat()" class="text-white/80 hover:text-white text-lg" style="background:none;border:none;cursor:pointer">✕</button>
    </div>
  </div>
  <div class="ai-drawer-body" id="ai-chat-box">
    <div class="flex justify-start pl-2 pr-8">
      <div class="bg-gray-50 border border-gray-100 p-3 rounded-2xl rounded-tl-none text-sm text-gray-600">你好！我是 AI 助手，可以帮你分析大棚环境数据。点击"环境分析"或直接提问。</div>
    </div>
  </div>
  <div class="ai-drawer-foot">
    <input type="text" id="ai-input" placeholder="输入你的问题..." class="flex-1 px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
    <button onclick="sendAIChat()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">发送</button>
  </div>
</div>

<!-- Main Content -->
<div class="main-wrap" id="main-wrap">

<!-- ===== Dashboard Panel ===== -->
<div class="panel active" id="panel-dashboard">
  <div class="flex items-center justify-between mb-5">
    <div>
      <h1 class="text-xl font-bold text-gray-800">实时监控</h1>
      <p class="text-xs text-gray-400 mt-1" id="val-time-sync">系统时间: --</p>
    </div>
    <div class="flex items-center gap-3">
      <span class="text-xs px-3 py-1 rounded-full bg-blue-50 text-blue-600 font-medium" id="mode-badge">自动模式</span>
    </div>
  </div>

  <!-- Sensor Cards -->
  <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4 mb-6">
    <div class="card card-sm sensor-card sc-temp">
      <div class="flex items-center justify-between"><span class="sensor-icon">🌡️</span><span class="s-badge" id="badge-temp">--</span></div>
      <div class="sensor-val" id="val-temp">-- °C</div>
      <div class="sensor-label">温度</div>
    </div>
    <div class="card card-sm sensor-card sc-hum">
      <div class="flex items-center justify-between"><span class="sensor-icon">💧</span><span class="s-badge" id="badge-hum">--</span></div>
      <div class="sensor-val" id="val-hum">-- %</div>
      <div class="sensor-label">湿度</div>
    </div>
    <div class="card card-sm sensor-card sc-lux">
      <div class="flex items-center justify-between"><span class="sensor-icon">☀️</span><span class="s-badge" id="badge-lux">--</span></div>
      <div class="sensor-val" id="val-lux">-- lx</div>
      <div class="sensor-label">光照</div>
    </div>
    <div class="card card-sm sensor-card sc-soil">
      <div class="flex items-center justify-between"><span class="sensor-icon">🌱</span><span class="s-badge" id="badge-soil">--</span></div>
      <div class="sensor-val" id="val-soil">-- %</div>
      <div class="sensor-label">土壤水分</div>
    </div>
    <div class="card card-sm sensor-card sc-co2">
      <div class="flex items-center justify-between"><span class="sensor-icon">☁️</span><span class="s-badge" id="badge-eco2">--</span></div>
      <div class="sensor-val" id="val-eco2">-- ppm</div>
      <div class="sensor-label">eCO2</div>
    </div>
    <div class="card card-sm sensor-card sc-tvoc">
      <div class="flex items-center justify-between"><span class="sensor-icon">🧪</span><span class="s-badge" id="badge-tvoc">--</span></div>
      <div class="sensor-val" id="val-tvoc">-- ppb</div>
      <div class="sensor-label">TVOC</div>
    </div>
  </div>

  <!-- Alert Panel -->
  <div class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">⚡ 环境预警</h3>
    <div id="alert-list"><div class="text-green-600 text-sm font-medium">✅ 环境状态良好</div></div>
  </div>

  <!-- Charts -->
  <div class="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
    <div class="card card-sm"><div class="chart-box" id="chart-th"></div></div>
    <div class="card card-sm"><div class="chart-box" id="chart-ls"></div></div>
    <div class="card card-sm"><div class="chart-box" id="chart-aq"></div></div>
  </div>

  <!-- Statistics Table -->
  <div class="card">
    <h3 class="text-sm font-bold text-gray-600 mb-3">📈 实时统计</h3>
    <div class="overflow-x-auto">
      <table class="w-full text-sm text-center">
        <thead><tr class="bg-slate-50 text-gray-500 text-xs">
          <th class="px-3 py-2 text-left font-semibold">指标</th>
          <th class="px-3 py-2 font-semibold">最小</th>
          <th class="px-3 py-2 font-semibold">最大</th>
          <th class="px-3 py-2 font-semibold">平均</th>
          <th class="px-3 py-2 font-semibold">当前</th>
        </tr></thead>
        <tbody id="stats-body"></tbody>
      </table>
    </div>
  </div>
</div>

<!-- ===== Control Panel ===== -->
<div class="panel" id="panel-control">
  <h1 class="text-xl font-bold text-gray-800 mb-5">设备控制</h1>

  <!-- Mode Switch -->
  <div class="card mb-6">
    <div class="flex items-center justify-between">
      <div>
        <h3 class="font-bold text-gray-800">运行模式</h3>
        <p class="text-xs text-gray-400 mt-1">自动模式下设备根据传感器数据自动调节</p>
      </div>
      <div class="flex items-center gap-3">
        <span class="text-sm font-bold" id="mode-text" style="color:#16a34a">自动</span>
        <label class="relative inline-flex items-center cursor-pointer">
          <input type="checkbox" id="cb-mode" class="sr-only peer" checked onchange="toggleMode()">
          <div class="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:bg-green-500 peer-checked:after:translate-x-6 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all after:shadow"></div>
        </label>
      </div>
    </div>
  </div>

  <!-- Device Cards -->
  <div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-6">
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-blue-50"><span>💧</span></div>
        <div><div class="font-bold text-gray-800 text-sm">水泵</div><div class="text-xs text-gray-400">土壤灌溉</div></div>
      </div>
      <div class="toggle disabled" id="cb-pump" onclick="toggleDevice('pump')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-yellow-50"><span>💡</span></div>
        <div><div class="font-bold text-gray-800 text-sm">补光灯</div><div class="text-xs text-gray-400">植物补光</div></div>
      </div>
      <div class="toggle disabled" id="cb-light" onclick="toggleDevice('light')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-red-50"><span>🔥</span></div>
        <div><div class="font-bold text-gray-800 text-sm">加热垫</div><div class="text-xs text-gray-400">温度调节</div></div>
      </div>
      <div class="toggle disabled" id="cb-heater" onclick="toggleDevice('heater')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-cyan-50"><span>🌀</span></div>
        <div><div class="font-bold text-gray-800 text-sm">排风扇</div><div class="text-xs text-gray-400">通风换气</div></div>
      </div>
      <div class="toggle disabled" id="cb-fan" onclick="toggleDevice('fan')"></div>
    </div>
  </div>

  <!-- Schedule -->
  <div class="card">
    <h3 class="font-bold text-gray-800 mb-4">⏰ 定时任务</h3>
    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
      <div class="bg-slate-50 rounded-xl p-4">
        <div class="flex items-center justify-between mb-3">
          <span class="font-semibold text-sm text-gray-700">🌀 定时通风</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="sched-fan-en" class="sr-only peer" onchange="saveSchedule()">
            <div class="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
        <div class="flex items-center gap-2 text-sm">
          <input type="time" id="sched-fan-start" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
          <span class="text-gray-400">至</span>
          <input type="time" id="sched-fan-end" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
        </div>
      </div>
      <div class="bg-slate-50 rounded-xl p-4">
        <div class="flex items-center justify-between mb-3">
          <span class="font-semibold text-sm text-gray-700">💡 定时补光</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="sched-light-en" class="sr-only peer" onchange="saveSchedule()">
            <div class="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
        <div class="flex items-center gap-2 text-sm">
          <input type="time" id="sched-light-start" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
          <span class="text-gray-400">至</span>
          <input type="time" id="sched-light-end" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
        </div>
      </div>
    </div>
  </div>
</div>

<!-- ===== History Panel ===== -->
<div class="panel" id="panel-history">
  <h1 class="text-xl font-bold text-gray-800 mb-5">历史数据</h1>

  <div class="card mb-6">
    <div class="flex flex-wrap items-center gap-3 mb-3">
      <select id="hist-date-select" class="px-3 py-2 rounded-lg border text-sm focus:ring-2 focus:ring-blue-400 outline-none">
        <option value="">加载中...</option>
      </select>
      <button onclick="loadHistoryData()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">查询数据</button>
      <button onclick="loadHistoryLog()" class="px-4 py-2 bg-gray-500 hover:bg-gray-600 text-white rounded-lg text-sm font-medium transition-colors">查看日志</button>
    </div>
    <div class="text-xs text-gray-400" id="storage-info">存储状态加载中...</div>
  </div>

  <!-- History Charts -->
  <div id="hist-charts-wrap" style="display:none" class="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
    <div class="card card-sm"><div class="chart-box" id="hist-chart-th"></div></div>
    <div class="card card-sm"><div class="chart-box" id="hist-chart-ls"></div></div>
    <div class="card card-sm"><div class="chart-box" id="hist-chart-aq"></div></div>
  </div>

  <!-- History Stats -->
  <div id="hist-stats-wrap" style="display:none" class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">当日统计摘要</h3>
    <div class="overflow-x-auto">
      <table class="w-full text-sm text-center">
        <thead><tr class="bg-slate-50 text-gray-500 text-xs">
          <th class="px-3 py-2 text-left font-semibold">指标</th>
          <th class="px-3 py-2 font-semibold">最小</th>
          <th class="px-3 py-2 font-semibold">最大</th>
          <th class="px-3 py-2 font-semibold">平均</th>
          <th class="px-3 py-2 font-semibold">数据点</th>
        </tr></thead>
        <tbody id="hist-stats-body"></tbody>
      </table>
    </div>
  </div>

  <!-- Log Viewer -->
  <div id="hist-log-wrap" style="display:none" class="card">
    <h3 class="text-sm font-bold text-gray-600 mb-3">操作日志</h3>
    <div id="hist-log-list" class="bg-slate-50 rounded-lg p-4 max-h-80 overflow-y-auto text-xs font-mono text-gray-700 space-y-1"></div>
  </div>
</div>

<!-- ===== QQ Bot Panel ===== -->
<div class="panel" id="panel-qqbot">
  <h1 class="text-xl font-bold text-gray-800 mb-5">QQ 机器人</h1>

  <div class="card mb-6">
    <div class="flex items-center justify-between mb-4">
      <div class="flex items-center gap-3">
        <span class="text-2xl">🤖</span>
        <div>
          <h3 class="font-bold text-gray-800">机器人配置</h3>
          <span class="text-xs font-medium" id="qqbot-status">未配置</span>
        </div>
      </div>
      <label class="relative inline-flex items-center cursor-pointer">
        <input type="checkbox" id="qqbot-enabled" class="sr-only peer" onchange="saveQQBotConfig()">
        <div class="w-12 h-6 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-6 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all after:shadow"></div>
      </label>
    </div>
    <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
      <div>
        <label class="block text-xs font-semibold text-gray-500 mb-1">AppID</label>
        <input type="text" id="qqbot-appid" placeholder="QQ Bot AppID" class="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
      </div>
      <div>
        <label class="block text-xs font-semibold text-gray-500 mb-1">AppSecret</label>
        <input type="password" id="qqbot-secret" placeholder="AppSecret" class="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
      </div>
      <div>
        <label class="block text-xs font-semibold text-gray-500 mb-1">用户 OpenID（可选）</label>
        <input type="text" id="qqbot-groupid" placeholder="主动推送用" class="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
      </div>
    </div>
    <div class="flex flex-wrap items-center gap-3">
      <button onclick="saveQQBotConfig()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">保存配置</button>
      <button onclick="testQQBot()" class="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg text-sm font-medium transition-colors">发送测试</button>
    </div>
  </div>

  <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
    <div class="card">
      <h3 class="text-sm font-bold text-gray-600 mb-3">Gateway 状态</h3>
      <div class="flex items-center gap-3 mb-3">
        <span class="w-3 h-3 rounded-full" id="gw-status-dot" style="background:#9ca3af"></span>
        <span class="text-sm font-medium" id="gw-status-text">未连接</span>
      </div>
      <p class="text-xs text-gray-400">Gateway 连接后，用户可私聊机器人查询大棚状态</p>
    </div>
    <div class="card">
      <h3 class="text-sm font-bold text-gray-600 mb-3">支持的指令</h3>
      <div class="space-y-2">
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">状态</code><span class="text-gray-600">查看环境数据</span></div>
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">告警</code><span class="text-gray-600">查看当前告警</span></div>
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">帮助</code><span class="text-gray-600">显示指令列表</span></div>
      </div>
    </div>
  </div>
</div>

<!-- ===== System Panel ===== -->
<div class="panel" id="panel-system">
  <h1 class="text-xl font-bold text-gray-800 mb-5">系统信息</h1>

  <!-- Sensor Status -->
  <div class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">传感器状态</h3>
    <div class="grid grid-cols-2 md:grid-cols-4 gap-3">
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-sht40"></span>
        <span class="text-sm text-gray-600">SHT40 温湿度</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-bh1750"></span>
        <span class="text-sm text-gray-600">BH1750 光照</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-sgp30"></span>
        <span class="text-sm text-gray-600">SGP30 空气</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot on" id="dot-soil"></span>
        <span class="text-sm text-gray-600">ADC 土壤</span>
      </div>
    </div>
  </div>

  <!-- System Info -->
  <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">内存使用</div>
      <div class="text-lg font-bold text-gray-800" id="val-heap">-- %</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">WiFi 信号</div>
      <div class="text-lg font-bold text-gray-800"><span id="val-rssi">--</span> <span class="text-sm font-normal" id="val-rssi-quality"></span></div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">运行时间</div>
      <div class="text-lg font-bold text-gray-800" id="val-uptime">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">芯片信息</div>
      <div class="text-sm font-bold text-gray-800"><span id="val-chip-rev">--</span> · <span id="val-cpu-cores">--</span> · <span id="val-cpu-freq">--</span></div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">网络</div>
      <div class="text-sm font-bold text-gray-800" id="val-ip">--</div>
      <div class="text-xs text-gray-400 mt-1" id="val-mac">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">SDK 版本</div>
      <div class="text-sm font-bold text-gray-800" id="val-sdk">--</div>
    </div>
  </div>
</div>

</div><!-- /main-wrap -->

<script>
(function(){
  // ===== Tab Management =====
  var currentTab = 'dashboard';
  window.switchTab = function(tab) {
    currentTab = tab;
    document.querySelectorAll('.panel').forEach(function(p){ p.classList.remove('active'); });
    document.getElementById('panel-' + tab).classList.add('active');
    document.querySelectorAll('.nav-item').forEach(function(n){
      n.classList.toggle('active', n.getAttribute('data-tab') === tab);
    });
    document.querySelectorAll('.tab-btn').forEach(function(n){
      n.classList.toggle('active', n.getAttribute('data-tab') === tab);
    });
    closeSidebar();
    if (tab === 'dashboard') { setTimeout(function(){ chartTH.resize(); chartLS.resize(); chartAQ.resize(); }, 50); }
    if (tab === 'history' && histChartTH) { setTimeout(function(){ histChartTH.resize(); histChartLS.resize(); histChartAQ.resize(); }, 50); }
  };
  window.openSidebar = function() {
    document.getElementById('sidebar').classList.add('open');
    document.getElementById('sidebar-overlay').classList.add('show');
  };
  window.closeSidebar = function() {
    document.getElementById('sidebar').classList.remove('open');
    document.getElementById('sidebar-overlay').classList.remove('show');
  };

  // ===== AI Chat Drawer =====
  var aiOpen = false;
  window.toggleAIChat = function() {
    aiOpen = !aiOpen;
    document.getElementById('ai-drawer').classList.toggle('open', aiOpen);
  };

  // ===== Charts =====
  var chartTH = echarts.init(document.getElementById('chart-th'));
  var chartLS = echarts.init(document.getElementById('chart-ls'));
  var chartAQ = echarts.init(document.getElementById('chart-aq'));

  function makeChartOpt(title, names, colors, yNames, yMin) {
    return {
      title:{text:title,left:'center',textStyle:{color:'#475569',fontSize:13}},
      tooltip:{trigger:'axis'},
      legend:{data:names,top:22,itemWidth:10,itemHeight:10,textStyle:{fontSize:11}},
      grid:{left:'14%',right:'14%',bottom:'10%',top:'28%'},
      xAxis:{type:'category',data:[]},
      yAxis:[
        {type:'value',name:yNames[0],min:yMin||undefined},
        {type:'value',name:yNames[1],position:'right',min:yNames[1]==='%'?0:undefined,max:yNames[1]==='%'?100:undefined}
      ],
      series:[
        {name:names[0],type:'line',data:[],smooth:true,itemStyle:{color:colors[0]},areaStyle:{opacity:0.08},symbol:'none'},
        {name:names[1],type:'line',yAxisIndex:1,data:[],smooth:true,itemStyle:{color:colors[1]},areaStyle:{opacity:0.08},symbol:'none'}
      ]
    };
  }
  chartTH.setOption(makeChartOpt('温湿度',['温度(°C)','湿度(%)'],['#ef4444','#3b82f6'],['°C','%']));
  chartLS.setOption(makeChartOpt('光照·土壤',['光照(lx)','土壤(%)'],['#f97316','#22c55e'],['lx','%']));
  chartAQ.setOption(makeChartOpt('空气质量',['eCO2(ppm)','TVOC(ppb)'],['#14b8a6','#a855f7'],['ppm','ppb'],400));

  // ===== WebSocket =====
  var gateway = 'ws://' + window.location.hostname + ':81/';
  var socket = null;
  var reconnectTimer = null;

  function setWsStatus(s) {
    var dots = [document.getElementById('ws-dot'), document.getElementById('ws-dot-mobile')];
    var text = document.getElementById('ws-status-text');
    dots.forEach(function(d){ if(!d)return; d.className='conn-dot '+(s==='on'?'on':s==='pending'?'pending':'off'); });
    if(text) text.innerText = s==='on'?'已连接':s==='pending'?'连接中':'未连接';
  }

  function connectWS() {
    if(reconnectTimer) clearTimeout(reconnectTimer);
    if(socket && socket.readyState!==WebSocket.CLOSED) socket.close();
    setWsStatus('pending');
    socket = new WebSocket(gateway);
    socket.onopen = function(){ setWsStatus('on'); };
    socket.onmessage = function(e){ try{updateUI(JSON.parse(e.data));}catch(err){} };
    socket.onerror = function(){ setWsStatus('off'); };
    socket.onclose = function(){ setWsStatus('off'); reconnectTimer=setTimeout(connectWS,3000); };
  }
  connectWS();
  document.addEventListener('visibilitychange',function(){ if(!document.hidden&&(!socket||socket.readyState===WebSocket.CLOSED))connectWS(); });

  // ===== Helpers =====
  function setBadge(id,val,low,warnH,high){
    var el=document.getElementById(id); if(!el)return;
    if(val==null){el.className='s-badge';el.innerText='--';return;}
    if(val<low){el.className='s-badge b-low';el.innerText='偏低';}
    else if(val>high){el.className='s-badge b-high';el.innerText='偏高';}
    else if(val>warnH){el.className='s-badge b-warn';el.innerText='注意';}
    else{el.className='s-badge b-ok';el.innerText='正常';}
  }

  function fmtUptime(s){
    var d=Math.floor(s/86400),h=Math.floor((s%86400)/3600),m=Math.floor((s%3600)/60),sec=s%60;
    if(d>0)return d+'天 '+h+'时 '+m+'分';
    if(h>0)return h+'时 '+m+'分 '+sec+'秒';
    return m+'分 '+sec+'秒';
  }

  function calcStats(arr){
    if(!arr||arr.length===0)return null;
    var min=Infinity,max=-Infinity,sum=0;
    for(var i=0;i<arr.length;i++){var v=parseFloat(arr[i]);if(v<min)min=v;if(v>max)max=v;sum+=v;}
    return{min:min,max:max,avg:sum/arr.length};
  }

  function buildStatsRows(history, current, tbodyId) {
    var metrics=[
      {key:'temp',label:'🌡️ 温度',unit:'°C',dec:1},
      {key:'hum',label:'💧 湿度',unit:'%',dec:1},
      {key:'lux',label:'☀️ 光照',unit:'lx',dec:0},
      {key:'soil',label:'🌱 土壤',unit:'%',dec:0},
      {key:'eco2',label:'☁️ eCO2',unit:'ppm',dec:0},
      {key:'tvoc',label:'🧪 TVOC',unit:'ppb',dec:0}
    ];
    var rows='';
    metrics.forEach(function(m){
      var s=calcStats(history[m.key]);
      var cur=(current&&current[m.key]!=null)?parseFloat(current[m.key]).toFixed(m.dec)+' '+m.unit:'--';
      if(s){
        rows+='<tr class="border-b border-slate-100 hover:bg-slate-50">';
        rows+='<td class="px-3 py-2 font-medium text-gray-700 text-left text-xs">'+m.label+'</td>';
        rows+='<td class="px-3 py-2 text-blue-600 font-mono text-xs">'+s.min.toFixed(m.dec)+'</td>';
        rows+='<td class="px-3 py-2 text-red-500 font-mono text-xs">'+s.max.toFixed(m.dec)+'</td>';
        rows+='<td class="px-3 py-2 text-gray-600 font-mono text-xs">'+s.avg.toFixed(m.dec)+'</td>';
        rows+='<td class="px-3 py-2 text-green-700 font-mono text-xs font-bold">'+cur+'</td>';
        rows+='</tr>';
      }
    });
    var el=document.getElementById(tbodyId);
    if(el&&rows) el.innerHTML=rows;
  }

  function updateAlerts(c){
    var a=[];
    if(c.temp!=null){
      if(c.temp>35)a.push({cls:'alert-danger',msg:'⚠️ 温度过高 ('+c.temp.toFixed(1)+'°C)'});
      else if(c.temp<10)a.push({cls:'alert-danger',msg:'🥶 温度过低 ('+c.temp.toFixed(1)+'°C)'});
      else if(c.temp>28)a.push({cls:'alert-warn',msg:'🌡️ 温度偏高 ('+c.temp.toFixed(1)+'°C)'});
      else if(c.temp<15)a.push({cls:'alert-warn',msg:'🌡️ 温度偏低 ('+c.temp.toFixed(1)+'°C)'});
    }
    if(c.hum!=null){
      if(c.hum>90)a.push({cls:'alert-danger',msg:'💧 湿度过高 ('+c.hum.toFixed(1)+'%)'});
      else if(c.hum<20)a.push({cls:'alert-warn',msg:'💧 湿度偏低 ('+c.hum.toFixed(1)+'%)'});
    }
    if(c.soil!=null){
      if(c.soil<20)a.push({cls:'alert-danger',msg:'🌱 土壤严重缺水 ('+c.soil+'%)'});
      else if(c.soil<30)a.push({cls:'alert-warn',msg:'🌱 土壤偏干 ('+c.soil+'%)'});
    }
    if(c.eco2!=null){
      if(c.eco2>1500)a.push({cls:'alert-danger',msg:'☁️ CO2超标 ('+c.eco2+'ppm)'});
      else if(c.eco2>1000)a.push({cls:'alert-warn',msg:'☁️ CO2偏高 ('+c.eco2+'ppm)'});
    }
    if(c.tvoc!=null){
      if(c.tvoc>660)a.push({cls:'alert-danger',msg:'🧪 TVOC超标 ('+c.tvoc+'ppb)'});
      else if(c.tvoc>220)a.push({cls:'alert-warn',msg:'🧪 TVOC偏高 ('+c.tvoc+'ppb)'});
    }
    var el=document.getElementById('alert-list');
    el.innerHTML=a.length===0?'<div class="text-green-600 text-sm font-medium">✅ 环境状态良好</div>':
      a.map(function(x){return '<div class="alert-item '+x.cls+'">'+x.msg+'</div>';}).join('');
  }

  // ===== Main UI Update =====
  function updateUI(data){
    if(data.ai_resp){
      var btn=document.getElementById('btn-ai-analyze');
      if(btn){btn.disabled=false;btn.innerText='环境分析';btn.classList.remove('opacity-50','cursor-not-allowed');}
      appendChat('ai',data.ai_resp);
    }
    if(data.current){
      var c=data.current;
      document.getElementById('val-temp').innerText=(c.temp!=null)?c.temp.toFixed(1)+' °C':'-- °C';
      document.getElementById('val-hum').innerText=(c.hum!=null)?c.hum.toFixed(1)+' %':'-- %';
      document.getElementById('val-lux').innerText=(c.lux!=null)?c.lux.toFixed(0)+' lx':'-- lx';
      document.getElementById('val-soil').innerText=(c.soil!=null)?c.soil+' %':'-- %';
      document.getElementById('val-eco2').innerText=(c.eco2!=null)?c.eco2+' ppm':'-- ppm';
      document.getElementById('val-tvoc').innerText=(c.tvoc!=null)?c.tvoc+' ppb':'-- ppb';
      setBadge('badge-temp',c.temp,10,28,35);
      setBadge('badge-hum',c.hum,20,80,90);
      setBadge('badge-lux',c.lux,200,800,2000);
      setBadge('badge-soil',c.soil,30,60,80);
      setBadge('badge-eco2',c.eco2,400,1000,1500);
      setBadge('badge-tvoc',c.tvoc,0,220,660);
      updateAlerts(c);
    }
    if(data.system){
      var sys=data.system;
      if(sys.heap_total&&sys.heap_free){
        var pct=((sys.heap_total-sys.heap_free)/sys.heap_total*100).toFixed(1);
        document.getElementById('val-heap').innerText=pct+' %';
      }
      if(sys.rssi){
        document.getElementById('val-rssi').innerText=sys.rssi+' dBm';
        var q=document.getElementById('val-rssi-quality');
        if(sys.rssi>=-50){q.innerText='极好';q.style.color='#16a34a';}
        else if(sys.rssi>=-70){q.innerText='良好';q.style.color='#2563eb';}
        else if(sys.rssi>=-80){q.innerText='一般';q.style.color='#d97706';}
        else{q.innerText='较差';q.style.color='#dc2626';}
      }
      if(sys.chip_rev)document.getElementById('val-chip-rev').innerText='Rev '+sys.chip_rev;
      if(sys.mac)document.getElementById('val-mac').innerText=sys.mac;
      if(sys.ip)document.getElementById('val-ip').innerText=sys.ip;
      if(sys.cpu_freq)document.getElementById('val-cpu-freq').innerText=sys.cpu_freq+'MHz';
      if(sys.cpu_cores)document.getElementById('val-cpu-cores').innerText=sys.cpu_cores+'核';
      if(sys.uptime!=null)document.getElementById('val-uptime').innerText=fmtUptime(sys.uptime);
      if(sys.sdk_ver)document.getElementById('val-sdk').innerText=sys.sdk_ver;
      if(sys.fs_total){
        var fsPct=(sys.fs_used/sys.fs_total*100).toFixed(1);
        var si=document.getElementById('storage-info');
        if(si) si.innerText='存储: '+(sys.fs_used/1024).toFixed(1)+'KB / '+(sys.fs_total/1024).toFixed(0)+'KB ('+fsPct+'%)';
      }
    }
    if(data.sensors){
      var ud=function(id,on){var el=document.getElementById(id);if(el)el.className='conn-dot '+(on?'on':'off');};
      ud('dot-sht40',data.sensors.sht40);ud('dot-bh1750',data.sensors.bh1750);
      ud('dot-sgp30',data.sensors.sgp30);ud('dot-soil',data.sensors.soil);
    }
    if(data.history&&data.history.time&&data.history.time.length>0){
      var h=data.history;
      chartTH.setOption({xAxis:{data:h.time},series:[{data:h.temp||[]},{data:h.hum||[]}]});
      chartLS.setOption({xAxis:{data:h.time},series:[{data:h.lux||[]},{data:h.soil||[]}]});
      chartAQ.setOption({xAxis:{data:h.time},series:[{data:h.eco2||[]},{data:h.tvoc||[]}]});
      buildStatsRows(h,data.current,'stats-body');
    }
    if(data.state){
      var isAuto=(data.state.mode==='auto');
      document.getElementById('cb-mode').checked=isAuto;
      var mt=document.getElementById('mode-text');
      mt.innerText=isAuto?'自动':'手动';
      mt.style.color=isAuto?'#16a34a':'#ea580c';
      document.getElementById('mode-badge').innerText=isAuto?'自动模式':'手动模式';
      document.getElementById('mode-badge').className='text-xs px-3 py-1 rounded-full font-medium '+(isAuto?'bg-green-50 text-green-600':'bg-orange-50 text-orange-600');
      ['pump','light','heater','fan'].forEach(function(dev){
        var el=document.getElementById('cb-'+dev);
        if(el){
          var on=(data.state[dev]===1);
          el.classList.toggle('on',on);
          el.classList.toggle('disabled',isAuto);
        }
      });
    }
    if(data.sched){
      document.getElementById('sched-fan-en').checked=data.sched.fan_en;
      document.getElementById('sched-fan-start').value=data.sched.fan_start||'00:00';
      document.getElementById('sched-fan-end').value=data.sched.fan_end||'00:00';
      document.getElementById('sched-light-en').checked=data.sched.light_en;
      document.getElementById('sched-light-start').value=data.sched.light_start||'00:00';
      document.getElementById('sched-light-end').value=data.sched.light_end||'00:00';
    }
    if(data.time){
      document.getElementById('val-time-sync').innerText='系统时间: '+data.time;
    }
  }

  // ===== Controls =====
  window.toggleMode=function(){
    var isAuto=document.getElementById('cb-mode').checked;
    if(socket&&socket.readyState===WebSocket.OPEN)socket.send(JSON.stringify({action:'set_mode',mode:isAuto?'auto':'manual'}));
  };
  window.toggleDevice=function(dev){
    var el=document.getElementById('cb-'+dev);
    if(el.classList.contains('disabled'))return;
    var isOn=!el.classList.contains('on');
    if(socket&&socket.readyState===WebSocket.OPEN)socket.send(JSON.stringify({action:'set_device',device:dev,state:isOn?1:0}));
  };
  window.saveSchedule=function(){
    if(!socket||socket.readyState!==WebSocket.OPEN)return;
    socket.send(JSON.stringify({
      action:'set_sched',
      fan_en:document.getElementById('sched-fan-en').checked,
      fan_start:document.getElementById('sched-fan-start').value||'00:00',
      fan_end:document.getElementById('sched-fan-end').value||'00:00',
      light_en:document.getElementById('sched-light-en').checked,
      light_start:document.getElementById('sched-light-start').value||'00:00',
      light_end:document.getElementById('sched-light-end').value||'00:00'
    }));
  };

  // ===== AI Chat =====
  function appendChat(role, msg) {
    var box = document.getElementById('ai-chat-box');
    var outer = document.createElement('div');
    outer.className = role === 'user' ? 'flex justify-end pr-2 pl-8 mb-3' : 'flex justify-start pl-2 pr-8 mb-3';
    var inner = document.createElement('div');
    inner.className = role === 'user'
      ? 'bg-blue-600 text-white p-3 rounded-2xl rounded-tr-none text-sm break-words'
      : 'bg-gray-50 border border-gray-100 p-3 rounded-2xl rounded-tl-none text-sm text-gray-700 leading-relaxed break-words whitespace-pre-wrap';
    inner.innerText = msg;
    outer.appendChild(inner);
    box.appendChild(outer);
    box.scrollTop = box.scrollHeight;
  }

  window.askAI = function() {
    if (socket && socket.readyState === WebSocket.OPEN) {
      var btn = document.getElementById('btn-ai-analyze');
      btn.disabled = true;
      btn.innerText = '分析中...';
      btn.classList.add('opacity-50', 'cursor-not-allowed');
      appendChat('user', '请根据当前环境数据分析大棚状态并给出建议。');
      socket.send(JSON.stringify({ action: 'ai_analyze' }));
    }
  };

  window.sendAIChat = function() {
    var input = document.getElementById('ai-input');
    var q = input.value.trim();
    if (!q || !socket || socket.readyState !== WebSocket.OPEN) return;
    appendChat('user', q);
    socket.send(JSON.stringify({ action: 'ai_chat', question: q }));
    input.value = '';
  };

  document.getElementById('ai-input') && document.getElementById('ai-input').addEventListener('keypress', function(e) {
    if (e.key === 'Enter') sendAIChat();
  });

  // ===== History =====
  var histChartTH=null,histChartLS=null,histChartAQ=null;
  function initHistCharts(){
    if(histChartTH)return;
    histChartTH=echarts.init(document.getElementById('hist-chart-th'));
    histChartLS=echarts.init(document.getElementById('hist-chart-ls'));
    histChartAQ=echarts.init(document.getElementById('hist-chart-aq'));
  }
  function loadAvailableDates(){
    fetch('/api/dates').then(function(r){return r.json();}).then(function(data){
      var sel=document.getElementById('hist-date-select');sel.innerHTML='';
      var dates=(data.data_dates||[]).sort().reverse();
      if(dates.length===0){sel.innerHTML='<option value="">暂无归档</option>';}
      else{dates.forEach(function(d){var o=document.createElement('option');o.value=d;o.text=d.substring(0,4)+'-'+d.substring(4,6)+'-'+d.substring(6,8);sel.appendChild(o);});}
      if(data.total_bytes){
        var si=document.getElementById('storage-info');
        if(si) si.innerText='存储: '+(data.used_bytes/1024).toFixed(1)+'KB / '+(data.total_bytes/1024).toFixed(0)+'KB | 归档: '+dates.length+'天';
      }
    }).catch(function(){});
  }
  setTimeout(loadAvailableDates,2000);

  window.loadHistoryData=function(){
    var date=document.getElementById('hist-date-select').value;if(!date)return;
    fetch('/api/history?date='+date).then(function(r){return r.json();}).then(function(data){
      if(data.error){alert(data.error);return;}
      document.getElementById('hist-charts-wrap').style.display='';
      document.getElementById('hist-stats-wrap').style.display='';
      document.getElementById('hist-log-wrap').style.display='none';
      initHistCharts();
      var dl=date.substring(0,4)+'-'+date.substring(4,6)+'-'+date.substring(6,8);
      histChartTH.setOption(makeChartOpt(dl+' 温湿度',['温度(°C)','湿度(%)'],['#ef4444','#3b82f6'],['°C','%']));
      histChartTH.setOption({xAxis:{data:data.time||[]},series:[{data:data.temp||[]},{data:data.hum||[]}]});
      histChartLS.setOption(makeChartOpt(dl+' 光照·土壤',['光照(lx)','土壤(%)'],['#f97316','#22c55e'],['lx','%']));
      histChartLS.setOption({xAxis:{data:data.time||[]},series:[{data:data.lux||[]},{data:data.soil||[]}]});
      histChartAQ.setOption(makeChartOpt(dl+' 空气质量',['eCO2(ppm)','TVOC(ppb)'],['#14b8a6','#a855f7'],['ppm','ppb'],400));
      histChartAQ.setOption({xAxis:{data:data.time||[]},series:[{data:data.eco2||[]},{data:data.tvoc||[]}]});
      var metrics=[{key:'temp',label:'🌡️ 温度',unit:'°C',dec:1},{key:'hum',label:'💧 湿度',unit:'%',dec:1},{key:'lux',label:'☀️ 光照',unit:'lx',dec:0},{key:'soil',label:'🌱 土壤',unit:'%',dec:0},{key:'eco2',label:'☁️ eCO2',unit:'ppm',dec:0},{key:'tvoc',label:'🧪 TVOC',unit:'ppb',dec:0}];
      var rows='';
      metrics.forEach(function(m){
        var s=calcStats(data[m.key]);if(!s)return;
        rows+='<tr class="border-b border-slate-100"><td class="px-3 py-2 text-left text-xs font-medium text-gray-700">'+m.label+'</td>';
        rows+='<td class="px-3 py-2 text-blue-600 font-mono text-xs">'+s.min.toFixed(m.dec)+'</td>';
        rows+='<td class="px-3 py-2 text-red-500 font-mono text-xs">'+s.max.toFixed(m.dec)+'</td>';
        rows+='<td class="px-3 py-2 text-gray-600 font-mono text-xs">'+s.avg.toFixed(m.dec)+'</td>';
        rows+='<td class="px-3 py-2 font-mono text-xs">'+(data[m.key]||[]).length+'</td></tr>';
      });
      document.getElementById('hist-stats-body').innerHTML=rows;
      setTimeout(function(){histChartTH.resize();histChartLS.resize();histChartAQ.resize();},100);
    }).catch(function(e){alert('查询失败: '+e.message);});
  };

  window.loadHistoryLog=function(){
    var date=document.getElementById('hist-date-select').value;if(!date)return;
    fetch('/api/log?date='+date).then(function(r){return r.json();}).then(function(data){
      if(data.error){alert(data.error);return;}
      document.getElementById('hist-charts-wrap').style.display='none';
      document.getElementById('hist-stats-wrap').style.display='none';
      document.getElementById('hist-log-wrap').style.display='';
      var list=document.getElementById('hist-log-list');
      if(!data.entries||data.entries.length===0){list.innerHTML='<div class="text-gray-400">当日无记录</div>';}
      else{list.innerHTML=data.entries.map(function(e){return '<div class="py-1 border-b border-slate-200">'+e.replace(/</g,'&lt;')+'</div>';}).join('');}
    }).catch(function(e){alert('查询失败: '+e.message);});
  };

  // ===== QQ Bot =====
  function loadQQBotConfig(){
    fetch('/api/qqbot/config').then(function(r){return r.json();}).then(function(d){
      document.getElementById('qqbot-appid').value=d.appId||'';
      document.getElementById('qqbot-groupid').value=d.userOpenId||'';
      document.getElementById('qqbot-enabled').checked=d.enabled||false;
      var st=document.getElementById('qqbot-status');
      if(!d.appId){st.innerText='未配置';st.style.color='#9ca3af';}
      else if(d.gatewayReady){st.innerText='Gateway 就绪';st.style.color='#16a34a';}
      else if(d.tokenValid){st.innerText='Token 有效';st.style.color='#2563eb';}
      else if(d.enabled){st.innerText='已启用';st.style.color='#2563eb';}
      else{st.innerText='已禁用';st.style.color='#ea580c';}
      var dots=[document.getElementById('gw-status-dot'),document.getElementById('gw-dot-side')];
      var gwt=document.getElementById('gw-status-text');
      var gws=document.getElementById('gw-status-side');
      if(d.gatewayReady){
        dots.forEach(function(x){if(x)x.style.background='#22c55e';});
        if(gwt)gwt.innerText='已连接';if(gws)gws.innerText='GW 在线';
      }else if(d.gatewayConnected){
        dots.forEach(function(x){if(x)x.style.background='#f59e0b';});
        if(gwt)gwt.innerText='连接中...';if(gws)gws.innerText='GW 连接中';
      }else if(d.enabled){
        dots.forEach(function(x){if(x)x.style.background='#ef4444';});
        if(gwt)gwt.innerText='未连接';if(gws)gws.innerText='GW 离线';
      }else{
        dots.forEach(function(x){if(x)x.style.background='#9ca3af';});
        if(gwt)gwt.innerText='未启用';if(gws)gws.innerText='GW 未启用';
      }
    }).catch(function(){});
  }
  setTimeout(loadQQBotConfig,3000);
  setInterval(loadQQBotConfig,15000);

  window.saveQQBotConfig=function(){
    var body={appId:document.getElementById('qqbot-appid').value,userOpenId:document.getElementById('qqbot-groupid').value,enabled:document.getElementById('qqbot-enabled').checked};
    var sec=document.getElementById('qqbot-secret').value;if(sec)body.appSecret=sec;
    fetch('/api/qqbot/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}).then(function(r){return r.json();}).then(function(d){
      if(d.ok){alert('配置已保存');loadQQBotConfig();}
    }).catch(function(e){alert('保存失败: '+e.message);});
  };
  window.testQQBot=function(){
    fetch('/api/qqbot/test').then(function(r){return r.json();}).then(function(d){
      alert(d.ok?'测试消息发送成功！':('失败: '+(d.error||'未知错误')));
    }).catch(function(e){alert('请求失败: '+e.message);});
  };

  // ===== Resize =====
  window.addEventListener('resize',function(){chartTH.resize();chartLS.resize();chartAQ.resize();});
})();
</script>
</body>
</html>
)rawliteral";
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1, user-scalable=yes'>
  <title>AI 大棚监控系统 · 优化版</title>
  <script src="https://cdn.tailwindcss.com"></script>
  <script src="https://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js"></script>
  <style>
    .toggle-switch:checked + div { background-color: #22c55e; }
    .toggle-switch:disabled + div { opacity: 0.5; cursor: not-allowed; }
    .status-dot { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 6px; }
    .status-connected { background-color: #22c55e; box-shadow: 0 0 8px #22c55e; }
    .status-disconnected { background-color: #ef4444; box-shadow: 0 0 8px #ef4444; }
    .status-connecting { background-color: #f59e0b; box-shadow: 0 0 8px #f59e0b; }
    .text-quality-excellent { color: #22c55e; }
    .text-quality-good { color: #84cc16; }
    .text-quality-fair { color: #f59e0b; }
    .text-quality-poor { color: #ef4444; }
    .status-badge { display: inline-block; padding: 1px 8px; border-radius: 9999px; font-size: 11px; font-weight: 600; }
    .badge-normal { background: #dcfce7; color: #16a34a; }
    .badge-high   { background: #fee2e2; color: #dc2626; }
    .badge-low    { background: #dbeafe; color: #2563eb; }
    .badge-warn   { background: #fef9c3; color: #ca8a04; }
    .alert-warn   { background: #fffbeb; border-left: 4px solid #f59e0b; padding: 8px 12px; border-radius: 4px; }
    .alert-danger { background: #fef2f2; border-left: 4px solid #ef4444; padding: 8px 12px; border-radius: 4px; }
  </style>
</head>
<body class="bg-slate-100 font-sans p-4 md:p-8 transition-colors duration-200">

  <div class="max-w-6xl mx-auto space-y-6">

    <!-- 标题栏 -->
    <div class="flex flex-col md:flex-row items-center justify-between bg-white rounded-2xl shadow-sm p-6">
      <div class="flex items-center space-x-3">
        <h1 class="text-3xl font-extrabold text-green-700 tracking-tight">🤖 AI 大棚核心控制</h1>
        <div id="connection-status" class="flex items-center text-sm font-medium text-gray-600 bg-gray-100 px-3 py-1 rounded-full">
          <span class="status-dot status-connecting" id="status-dot"></span>
          <span id="status-text">连接中...</span>
        </div>
      </div>
      <a href='/reset' class="mt-4 md:mt-0 px-4 py-2 bg-red-500 hover:bg-red-600 text-white rounded-lg shadow transition-colors text-sm font-medium" onclick="return confirm('确定要清除 WiFi 配置并重启吗？')">⚠️ 重置WiFi</a>
    </div>

    <!-- 👇 修改：实时环境数据卡片扩容为 6 项 -->
    <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4">
      <div class="bg-white p-4 rounded-2xl shadow-sm text-center flex flex-col items-center justify-center hover:shadow-md transition-shadow">
        <div class="text-gray-500 text-xs font-semibold mb-1">🌡️ 温度</div>
        <div class="text-2xl font-bold text-red-500" id="val-temp">-- °C</div>
        <div class="mt-1"><span class="status-badge" id="badge-temp">--</span></div>
      </div>
      <div class="bg-white p-4 rounded-2xl shadow-sm text-center flex flex-col items-center justify-center hover:shadow-md transition-shadow">
        <div class="text-gray-500 text-xs font-semibold mb-1">💧 湿度</div>
        <div class="text-2xl font-bold text-blue-500" id="val-hum">-- %</div>
        <div class="mt-1"><span class="status-badge" id="badge-hum">--</span></div>
      </div>
      <div class="bg-white p-4 rounded-2xl shadow-sm text-center flex flex-col items-center justify-center hover:shadow-md transition-shadow">
        <div class="text-gray-500 text-xs font-semibold mb-1">☀️ 光照</div>
        <div class="text-2xl font-bold text-orange-400" id="val-lux">-- lx</div>
        <div class="mt-1"><span class="status-badge" id="badge-lux">--</span></div>
      </div>
      <div class="bg-white p-4 rounded-2xl shadow-sm text-center flex flex-col items-center justify-center hover:shadow-md transition-shadow">
        <div class="text-gray-500 text-xs font-semibold mb-1">🌱 土壤</div>
        <div class="text-2xl font-bold text-green-600" id="val-soil">-- %</div>
        <div class="mt-1"><span class="status-badge" id="badge-soil">--</span></div>
      </div>
      <div class="bg-white p-4 rounded-2xl shadow-sm text-center flex flex-col items-center justify-center hover:shadow-md transition-shadow">
        <div class="text-gray-500 text-xs font-semibold mb-1">☁️ CO2 (eCO2)</div>
        <div class="text-2xl font-bold text-teal-500" id="val-eco2">-- ppm</div>
        <div class="mt-1"><span class="status-badge" id="badge-eco2">--</span></div>
      </div>
      <div class="bg-white p-4 rounded-2xl shadow-sm text-center flex flex-col items-center justify-center hover:shadow-md transition-shadow">
        <div class="text-gray-500 text-xs font-semibold mb-1">🧪 有机挥发物</div>
        <div class="text-2xl font-bold text-purple-500" id="val-tvoc">-- ppb</div>
        <div class="mt-1"><span class="status-badge" id="badge-tvoc">--</span></div>
      </div>
    </div>

    <!-- 系统状态卡片 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <h2 class="text-xl font-bold text-gray-800 mb-4 border-b pb-3">⚙️ 系统状态</h2>
      <div class="grid grid-cols-2 md:grid-cols-4 gap-x-6 gap-y-4 text-sm">
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">内存占用:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-heap">-- %</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">WiFi信号:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-rssi">-- dBm</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">信号质量:</span>
          <span class="font-bold" id="val-rssi-quality">--</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">芯片版本:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-chip-rev">--</span>
        </div>
        <div class="flex justify-between border-b pb-1 col-span-2 md:col-span-4">
          <span class="font-semibold text-gray-600">MAC地址:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-mac">--:--:--:--:--:--</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">IP地址:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-ip">--</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">CPU频率:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-cpu-freq">-- MHz</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">CPU核心:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-cpu-cores">--</span>
        </div>
        <div class="flex justify-between border-b pb-1">
          <span class="font-semibold text-gray-600">运行时间:</span>
          <span class="font-mono text-gray-800 font-bold" id="val-uptime">--</span>
        </div>
        <div class="flex justify-between border-b pb-1 col-span-2 md:col-span-4">
          <span class="font-semibold text-gray-600">SDK版本:</span>
          <span class="font-mono text-gray-800 text-xs font-bold" id="val-sdk">--</span>
        </div>
      </div>
    </div>

    <!-- 传感器在线状态 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <h2 class="text-xl font-bold text-gray-800 mb-4 border-b pb-3">📡 传感器在线状态</h2>
      <div class="grid grid-cols-2 md:grid-cols-4 gap-4">
        <div class="flex items-center gap-3 p-3 rounded-lg bg-slate-50">
          <span id="dot-sht40" class="status-dot status-disconnected"></span>
          <div>
            <div class="font-semibold text-sm">SHT40</div>
            <div class="text-xs text-gray-500">温湿度</div>
          </div>
        </div>
        <div class="flex items-center gap-3 p-3 rounded-lg bg-slate-50">
          <span id="dot-bh1750" class="status-dot status-disconnected"></span>
          <div>
            <div class="font-semibold text-sm">BH1750</div>
            <div class="text-xs text-gray-500">光照</div>
          </div>
        </div>
        <div class="flex items-center gap-3 p-3 rounded-lg bg-slate-50">
          <span id="dot-sgp30" class="status-dot status-disconnected"></span>
          <div>
            <div class="font-semibold text-sm">SGP30</div>
            <div class="text-xs text-gray-500">空气质量</div>
          </div>
        </div>
        <div class="flex items-center gap-3 p-3 rounded-lg bg-slate-50">
          <span id="dot-soil" class="status-dot status-disconnected"></span>
          <div>
            <div class="font-semibold text-sm">土壤</div>
            <div class="text-xs text-gray-500">湿度ADC</div>
          </div>
        </div>
      </div>
    </div>

    <!-- 设备控制面板 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <div class="flex items-center justify-between mb-6 border-b pb-4">
        <h2 class="text-xl font-bold text-gray-800">🎛️ 设备控制面板</h2>
        <label class="relative inline-flex items-center cursor-pointer">
          <input type="checkbox" id="cb-mode" class="sr-only peer toggle-switch" onchange="toggleMode()">
          <div class="w-14 h-7 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-6 after:w-6 after:transition-all"></div>
          <span class="ml-3 font-bold text-lg w-28 text-left" id="mode-text">获取中...</span>
        </label>
      </div>

      <div class="grid grid-cols-2 md:grid-cols-4 gap-4">
        <div class="flex flex-col items-center p-4 bg-slate-50 rounded-xl border border-slate-100">
          <span class="font-bold text-gray-700 mb-3 text-lg">💦 水泵</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="cb-pump" class="sr-only peer toggle-switch" onchange="toggleDevice('pump')">
            <div class="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all"></div>
          </label>
        </div>
        <div class="flex flex-col items-center p-4 bg-slate-50 rounded-xl border border-slate-100">
          <span class="font-bold text-gray-700 mb-3 text-lg">💡 补光灯</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="cb-light" class="sr-only peer toggle-switch" onchange="toggleDevice('light')">
            <div class="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all"></div>
          </label>
        </div>
        <div class="flex flex-col items-center p-4 bg-slate-50 rounded-xl border border-slate-100">
          <span class="font-bold text-gray-700 mb-3 text-lg">🔥 加热垫</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="cb-heater" class="sr-only peer toggle-switch" onchange="toggleDevice('heater')">
            <div class="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all"></div>
          </label>
        </div>
        <div class="flex flex-col items-center p-4 bg-slate-50 rounded-xl border border-slate-100">
          <span class="font-bold text-gray-700 mb-3 text-lg">🌬️ 风扇</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="cb-fan" class="sr-only peer toggle-switch" onchange="toggleDevice('fan')">
            <div class="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all"></div>
          </label>
        </div>
      </div>
    </div>

    <!-- 环境预警 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <h2 class="text-xl font-bold text-gray-800 mb-4 border-b pb-3">🔔 环境预警</h2>
      <div id="alert-list" class="space-y-2 text-sm"><div class="text-gray-400">等待数据...</div></div>
    </div>

    <!-- 历史统计摘要 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <h2 class="text-xl font-bold text-gray-800 mb-4 border-b pb-3">📊 历史统计摘要（近60条）</h2>
      <div class="overflow-x-auto">
        <table class="min-w-full text-sm text-center">
          <thead>
            <tr class="bg-slate-50 text-gray-600">
              <th class="px-3 py-2 font-semibold text-left">指标</th>
              <th class="px-3 py-2 font-semibold">最小值</th>
              <th class="px-3 py-2 font-semibold">最大值</th>
              <th class="px-3 py-2 font-semibold">平均值</th>
              <th class="px-3 py-2 font-semibold">当前值</th>
            </tr>
          </thead>
          <tbody id="stats-body">
            <tr><td colspan="5" class="py-4 text-gray-400">等待历史数据...</td></tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- 任务调度与定时任务面板 -->
    <div class="bg-white rounded-2xl shadow-sm p-6 overflow-hidden">
      <div class="flex items-center justify-between mb-4 border-b pb-3">
        <h2 class="text-xl font-bold text-gray-800 flex items-center">
          <span class="mr-2">📅</span> 定时任务调度器
        </h2>
        <span class="text-xs font-mono text-gray-400" id="val-time-sync">时间同步中...</span>
      </div>
      <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
        <!-- 抽风/风扇定时 -->
        <div class="p-4 bg-slate-50 rounded-xl border border-slate-100">
          <div class="flex justify-between items-center mb-3">
            <span class="font-bold text-gray-700">🌬️ 强制通风 (每日)</span>
            <label class="relative inline-flex items-center cursor-pointer">
              <input type="checkbox" id="sched-fan-en" class="sr-only peer toggle-switch" onchange="saveSchedule()">
              <div class="w-10 h-5 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
            </label>
          </div>
          <div class="flex items-center space-x-2 text-sm text-gray-600">
            <input type="time" id="sched-fan-start" class="px-2 py-1 rounded border border-gray-200 focus:ring-1 focus:ring-blue-400 outline-none" onchange="saveSchedule()">
            <span>至</span>
            <input type="time" id="sched-fan-end" class="px-2 py-1 rounded border border-gray-200 focus:ring-1 focus:ring-blue-400 outline-none" onchange="saveSchedule()">
          </div>
        </div>
        <!-- 补光定时 -->
        <div class="p-4 bg-slate-50 rounded-xl border border-slate-100">
          <div class="flex justify-between items-center mb-3">
            <span class="font-bold text-gray-700">💡 补光计划 (每日)</span>
            <label class="relative inline-flex items-center cursor-pointer">
              <input type="checkbox" id="sched-light-en" class="sr-only peer toggle-switch" onchange="saveSchedule()">
              <div class="w-10 h-5 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
            </label>
          </div>
          <div class="flex items-center space-x-2 text-sm text-gray-600">
            <input type="time" id="sched-light-start" class="px-2 py-1 rounded border border-gray-200 focus:ring-1 focus:ring-blue-400 outline-none" onchange="saveSchedule()">
            <span>至</span>
            <input type="time" id="sched-light-end" class="px-2 py-1 rounded border border-gray-200 focus:ring-1 focus:ring-blue-400 outline-none" onchange="saveSchedule()">
          </div>
        </div>
      </div>
      <div class="mt-4 text-xs text-gray-400 bg-blue-50 p-2 rounded-lg">
        * 任务在“自动模式”下生效。手动模式将忽略定时冲突。系统时间由 NTP 自动同步。
      </div>
    </div>

    <!-- 👇 修改：历史曲线 (变更为 3 个图表排排坐) -->
    <div class="grid grid-cols-1 lg:grid-cols-3 gap-6">
      <div class="bg-white rounded-2xl shadow-sm p-4 h-80 w-full" id="chart-th"></div>
      <div class="bg-white rounded-2xl shadow-sm p-4 h-80 w-full" id="chart-ls"></div>
      <div class="bg-white rounded-2xl shadow-sm p-4 h-80 w-full" id="chart-aq"></div>
    </div>

    <!-- QQ Bot 配置面板 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <div class="flex items-center justify-between mb-4 border-b pb-3">
        <h2 class="text-xl font-bold text-gray-800">🤖 QQ 机器人</h2>
        <div class="flex items-center gap-2">
          <span class="text-xs font-medium" id="qqbot-status">未配置</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="qqbot-enabled" class="sr-only peer toggle-switch" onchange="saveQQBotConfig()">
            <div class="w-10 h-5 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-gray-300 after:border after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
      </div>
      <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
        <div>
          <label class="block text-xs font-semibold text-gray-500 mb-1">AppID</label>
          <input type="text" id="qqbot-appid" placeholder="输入 QQ Bot AppID" class="w-full px-3 py-2 text-sm rounded-lg border border-gray-200 focus:ring-2 focus:ring-blue-400 outline-none">
        </div>
        <div>
          <label class="block text-xs font-semibold text-gray-500 mb-1">AppSecret</label>
          <input type="password" id="qqbot-secret" placeholder="输入 AppSecret" class="w-full px-3 py-2 text-sm rounded-lg border border-gray-200 focus:ring-2 focus:ring-blue-400 outline-none">
        </div>
        <div>
          <label class="block text-xs font-semibold text-gray-500 mb-1">用户 OpenID（可选，主动推送用）</label>
          <input type="text" id="qqbot-groupid" placeholder="目标用户的 openid" class="w-full px-3 py-2 text-sm rounded-lg border border-gray-200 focus:ring-2 focus:ring-blue-400 outline-none">
        </div>
      </div>
      <!-- Gateway 状态 & 指令说明 -->
      <div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-4">
        <div class="bg-slate-50 rounded-lg p-3">
          <div class="text-xs font-semibold text-gray-500 mb-2">Gateway 状态</div>
          <div class="flex items-center gap-2">
            <span class="w-2 h-2 rounded-full" id="gw-status-dot" style="background:#9ca3af"></span>
            <span class="text-sm" id="gw-status-text">未连接</span>
          </div>
        </div>
        <div class="bg-slate-50 rounded-lg p-3">
          <div class="text-xs font-semibold text-gray-500 mb-2">用户可发送的指令</div>
          <div class="text-xs text-gray-600 space-y-1">
            <div><code class="bg-white px-1.5 py-0.5 rounded border text-blue-600">状态</code> 查看环境数据</div>
            <div><code class="bg-white px-1.5 py-0.5 rounded border text-blue-600">告警</code> 查看当前告警</div>
            <div><code class="bg-white px-1.5 py-0.5 rounded border text-blue-600">帮助</code> 显示指令列表</div>
          </div>
        </div>
      </div>
      <div class="flex items-center gap-3">
        <button onclick="saveQQBotConfig()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">保存配置</button>
        <button onclick="testQQBot()" class="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg text-sm font-medium transition-colors">发送测试</button>
        <span class="text-xs text-gray-400">* 用户私聊机器人即可查询状态；环境异常自动推送告警</span>
      </div>
    </div>

    <!-- 历史数据查询面板 -->
    <div class="bg-white rounded-2xl shadow-sm p-6">
      <div class="flex flex-col md:flex-row items-center justify-between mb-4 border-b pb-3">
        <h2 class="text-xl font-bold text-gray-800">📁 历史数据归档</h2>
        <div class="flex items-center gap-3 mt-3 md:mt-0">
          <select id="hist-date-select" class="px-3 py-2 rounded-lg border border-gray-200 text-sm focus:ring-2 focus:ring-blue-400 outline-none">
            <option value="">加载中...</option>
          </select>
          <button onclick="loadHistoryData()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">查询数据</button>
          <button onclick="loadHistoryLog()" class="px-4 py-2 bg-gray-500 hover:bg-gray-600 text-white rounded-lg text-sm font-medium transition-colors">查看日志</button>
        </div>
      </div>
      <div class="text-xs text-gray-400 mb-4" id="storage-info">存储状态加载中...</div>
      <!-- 归档图表 -->
      <div class="grid grid-cols-1 lg:grid-cols-3 gap-6 mb-6" id="hist-charts-wrap" style="display:none;">
        <div class="bg-slate-50 rounded-xl p-3 h-72" id="hist-chart-th"></div>
        <div class="bg-slate-50 rounded-xl p-3 h-72" id="hist-chart-ls"></div>
        <div class="bg-slate-50 rounded-xl p-3 h-72" id="hist-chart-aq"></div>
      </div>
      <!-- 归档统计摘要 -->
      <div id="hist-stats-wrap" style="display:none;" class="mb-4">
        <h3 class="text-sm font-bold text-gray-600 mb-2">当日统计摘要</h3>
        <div class="overflow-x-auto">
          <table class="min-w-full text-sm text-center">
            <thead><tr class="bg-slate-50 text-gray-600">
              <th class="px-3 py-2 font-semibold text-left">指标</th>
              <th class="px-3 py-2 font-semibold">最小值</th>
              <th class="px-3 py-2 font-semibold">最大值</th>
              <th class="px-3 py-2 font-semibold">平均值</th>
              <th class="px-3 py-2 font-semibold">数据点数</th>
            </tr></thead>
            <tbody id="hist-stats-body"></tbody>
          </table>
        </div>
      </div>
      <!-- 操作日志 -->
      <div id="hist-log-wrap" style="display:none;">
        <h3 class="text-sm font-bold text-gray-600 mb-2">操作日志</h3>
        <div id="hist-log-list" class="bg-slate-50 rounded-lg p-4 max-h-64 overflow-y-auto text-xs font-mono text-gray-700 space-y-1"></div>
      </div>
    </div>
  </div>

  <script>
    (function() {
      // ----- 初始化图表 -----
      var chartTH = echarts.init(document.getElementById('chart-th'));
      var chartLS = echarts.init(document.getElementById('chart-ls'));
      var chartAQ = echarts.init(document.getElementById('chart-aq')); // 👇 新增图表

      var optTH = {
        title: { text: '温湿度历史', left: 'center', textStyle: { color: '#475569', fontSize: 14 } },
        tooltip: { trigger: 'axis' },
        legend: { data: ['温度(°C)', '湿度(%)'], top: 25, itemWidth: 10, itemHeight: 10 },
        grid: { left: '15%', right: '15%', bottom: '10%', top: '25%' },
        xAxis: { type: 'category', data: [] },
        yAxis:[
          { type: 'value', name: '°C' },
          { type: 'value', name: '%', position: 'right', min: 0, max: 100 }
        ],
        series:[
          { name: '温度(°C)', type: 'line', data: [], smooth: true, itemStyle: { color: '#ef4444' }, areaStyle: { opacity: 0.1 } },
          { name: '湿度(%)', type: 'line', yAxisIndex: 1, data:[], smooth: true, itemStyle: { color: '#3b82f6' }, areaStyle: { opacity: 0.1 } }
        ]
      };
      
      var optLS = {
        title: { text: '光照与土壤水分', left: 'center', textStyle: { color: '#475569', fontSize: 14 } },
        tooltip: { trigger: 'axis' },
        legend: { data: ['光照(lx)', '土壤(%)'], top: 25, itemWidth: 10, itemHeight: 10 },
        grid: { left: '15%', right: '15%', bottom: '10%', top: '25%' },
        xAxis: { type: 'category', data: [] },
        yAxis:[
          { type: 'value', name: 'lx' },
          { type: 'value', name: '%', position: 'right', min: 0, max: 100 }
        ],
        series:[
          { name: '光照(lx)', type: 'line', data: [], smooth: true, itemStyle: { color: '#f97316' }, areaStyle: { opacity: 0.1 } },
          { name: '土壤(%)', type: 'line', yAxisIndex: 1, data:[], smooth: true, itemStyle: { color: '#22c55e' }, areaStyle: { opacity: 0.1 } }
        ]
      };

      // 👇 新增空气质量图表配置
      var optAQ = {
        title: { text: '空气质量 (SGP30)', left: 'center', textStyle: { color: '#475569', fontSize: 14 } },
        tooltip: { trigger: 'axis' },
        legend: { data:['eCO2(ppm)', 'TVOC(ppb)'], top: 25, itemWidth: 10, itemHeight: 10 },
        grid: { left: '15%', right: '15%', bottom: '10%', top: '25%' },
        xAxis: { type: 'category', data:[] },
        yAxis:[
          { type: 'value', name: 'ppm', min: 400 },
          { type: 'value', name: 'ppb', position: 'right' }
        ],
        series:[
          { name: 'eCO2(ppm)', type: 'line', data: [], smooth: true, itemStyle: { color: '#14b8a6' }, areaStyle: { opacity: 0.1 } },
          { name: 'TVOC(ppb)', type: 'line', yAxisIndex: 1, data:[], smooth: true, itemStyle: { color: '#a855f7' }, areaStyle: { opacity: 0.1 } }
        ]
      };

      chartTH.setOption(optTH);
      chartLS.setOption(optLS);
      chartAQ.setOption(optAQ); // 挂载新图表

      // ----- WebSocket 管理 -----
      var gateway = `ws://${window.location.hostname}:81/`;
      var socket = null;
      var reconnectTimer = null;

      function setConnectionStatus(state) {
        var dot = document.getElementById('status-dot');
        var text = document.getElementById('status-text');
        dot.classList.remove('status-connected', 'status-disconnected', 'status-connecting');
        if (state === 'connected') {
          dot.classList.add('status-connected'); text.innerText = '已连接';
        } else if (state === 'disconnected') {
          dot.classList.add('status-disconnected'); text.innerText = '未连接';
        } else {
          dot.classList.add('status-connecting'); text.innerText = '连接中...';
        }
      }

      function connectWebSocket() {
        if (reconnectTimer) clearTimeout(reconnectTimer);
        if (socket && socket.readyState !== WebSocket.CLOSED) socket.close();
        setConnectionStatus('connecting');
        socket = new WebSocket(gateway);

        socket.onopen = function() { setConnectionStatus('connected'); };
        socket.onmessage = function(event) { try { updateUI(JSON.parse(event.data)); } catch(e) { console.warn('WebSocket parse error:', e); } };
        socket.onerror = function() { setConnectionStatus('disconnected'); };
        socket.onclose = function() {
          setConnectionStatus('disconnected');
          reconnectTimer = setTimeout(connectWebSocket, 3000);
        };
      }
      connectWebSocket();
      document.addEventListener('visibilitychange', function() {
        if (!document.hidden && (!socket || socket.readyState === WebSocket.CLOSED)) connectWebSocket();
      });

      // ----- 工具函数 -----
      function setSensorBadge(id, val, lowT, warnHigh, highT) {
        var el = document.getElementById(id);
        if (!el) return;
        if (val === undefined || val === null) { el.className = 'status-badge'; el.innerText = '--'; return; }
        if (val < lowT)       { el.className = 'status-badge badge-low';    el.innerText = '偏低'; }
        else if (val > highT) { el.className = 'status-badge badge-high';   el.innerText = '偏高'; }
        else if (val > warnHigh) { el.className = 'status-badge badge-warn'; el.innerText = '注意'; }
        else                  { el.className = 'status-badge badge-normal'; el.innerText = '正常'; }
      }

      function formatUptime(sec) {
        var d = Math.floor(sec / 86400), h = Math.floor((sec % 86400) / 3600),
            m = Math.floor((sec % 3600) / 60), s = sec % 60;
        if (d > 0) return d + '天 ' + h + '时 ' + m + '分';
        if (h > 0) return h + '时 ' + m + '分 ' + s + '秒';
        return m + '分 ' + s + '秒';
      }

      function calcStats(arr) {
        if (!arr || arr.length === 0) return null;
        var min = Infinity, max = -Infinity, sum = 0;
        for (var i = 0; i < arr.length; i++) {
          var v = parseFloat(arr[i]);
          if (v < min) min = v; if (v > max) max = v; sum += v;
        }
        return { min: min, max: max, avg: sum / arr.length };
      }

      function updateStats(history, current) {
        var metrics = [
          { key: 'temp', label: '🌡️ 温度',  unit: '°C',  dec: 1 },
          { key: 'hum',  label: '💧 湿度',  unit: '%',   dec: 1 },
          { key: 'lux',  label: '☀️ 光照',  unit: 'lx',  dec: 0 },
          { key: 'soil', label: '🌱 土壤',  unit: '%',   dec: 0 },
          { key: 'eco2', label: '☁️ eCO2', unit: 'ppm', dec: 0 },
          { key: 'tvoc', label: '🧪 TVOC', unit: 'ppb', dec: 0 }
        ];
        var rows = '';
        metrics.forEach(function(m) {
          var s = calcStats(history[m.key]);
          var cur = (current && current[m.key] !== undefined) ? parseFloat(current[m.key]).toFixed(m.dec) + ' ' + m.unit : '--';
          if (s) {
            rows += '<tr class="border-b border-slate-100 hover:bg-slate-50">';
            rows += '<td class="px-3 py-2 font-medium text-gray-700 text-left">' + m.label + '</td>';
            rows += '<td class="px-3 py-2 text-blue-600 font-mono">' + s.min.toFixed(m.dec) + ' ' + m.unit + '</td>';
            rows += '<td class="px-3 py-2 text-red-500 font-mono">'  + s.max.toFixed(m.dec) + ' ' + m.unit + '</td>';
            rows += '<td class="px-3 py-2 text-gray-600 font-mono">' + s.avg.toFixed(m.dec) + ' ' + m.unit + '</td>';
            rows += '<td class="px-3 py-2 text-green-700 font-mono font-bold">' + cur + '</td>';
            rows += '</tr>';
          }
        });
        if (rows) document.getElementById('stats-body').innerHTML = rows;
      }

      function updateAlerts(current) {
        var alerts = [];
        if (current.temp !== undefined) {
          if (current.temp > 35)      alerts.push({ cls: 'alert-danger', msg: '⚠️ 温度过高 (' + current.temp.toFixed(1) + '°C)，请检查通风！' });
          else if (current.temp < 10) alerts.push({ cls: 'alert-danger', msg: '🥶 温度过低 (' + current.temp.toFixed(1) + '°C)，请检查加热！' });
          else if (current.temp > 28) alerts.push({ cls: 'alert-warn',   msg: '🌡️ 温度偏高 (' + current.temp.toFixed(1) + '°C)' });
          else if (current.temp < 15) alerts.push({ cls: 'alert-warn',   msg: '🌡️ 温度偏低 (' + current.temp.toFixed(1) + '°C)' });
        }
        if (current.hum !== undefined) {
          if (current.hum > 90)       alerts.push({ cls: 'alert-danger', msg: '💧 湿度过高 (' + current.hum.toFixed(1) + '%)，有霉变风险！' });
          else if (current.hum < 20)  alerts.push({ cls: 'alert-warn',   msg: '💧 湿度偏低 (' + current.hum.toFixed(1) + '%)' });
        }
        if (current.soil !== undefined) {
          if (current.soil < 20)      alerts.push({ cls: 'alert-danger', msg: '🌱 土壤严重缺水 (' + current.soil + '%)，请立即浇水！' });
          else if (current.soil < 30) alerts.push({ cls: 'alert-warn',   msg: '🌱 土壤水分偏低 (' + current.soil + '%)' });
          else if (current.soil > 80) alerts.push({ cls: 'alert-warn',   msg: '🌱 土壤水分偏高 (' + current.soil + '%)' });
        }
        if (current.eco2 !== undefined) {
          if (current.eco2 > 1500)     alerts.push({ cls: 'alert-danger', msg: '☁️ CO2严重超标 (' + current.eco2 + 'ppm)，需紧急通风！' });
          else if (current.eco2 > 1000) alerts.push({ cls: 'alert-warn', msg: '☁️ CO2浓度偏高 (' + current.eco2 + 'ppm)' });
        }
        if (current.tvoc !== undefined) {
          if (current.tvoc > 660)      alerts.push({ cls: 'alert-danger', msg: '🧪 有机挥发物严重超标 (' + current.tvoc + 'ppb)！' });
          else if (current.tvoc > 220) alerts.push({ cls: 'alert-warn',   msg: '🧪 有机挥发物偏高 (' + current.tvoc + 'ppb)' });
        }
        var list = document.getElementById('alert-list');
        if (alerts.length === 0) {
          list.innerHTML = '<div class="text-green-600 font-medium">✅ 环境状态良好，无预警</div>';
        } else {
          list.innerHTML = alerts.map(function(a) { return '<div class="' + a.cls + '">' + a.msg + '</div>'; }).join('');
        }
      }

      // ----- UI 更新函数 -----
      function updateUI(data) {
        if (data.current) {
          document.getElementById('val-temp').innerText = (data.current.temp !== undefined) ? data.current.temp.toFixed(1) + ' °C' : '-- °C';
          document.getElementById('val-hum').innerText  = (data.current.hum  !== undefined) ? data.current.hum.toFixed(1)  + ' %'  : '-- %';
          document.getElementById('val-lux').innerText  = (data.current.lux  !== undefined) ? data.current.lux.toFixed(0)  + ' lx' : '-- lx';
          document.getElementById('val-soil').innerText = (data.current.soil !== undefined) ? data.current.soil + ' %'             : '-- %';
          document.getElementById('val-eco2').innerText = (data.current.eco2 !== undefined) ? data.current.eco2 + ' ppm'           : '-- ppm';
          document.getElementById('val-tvoc').innerText = (data.current.tvoc !== undefined) ? data.current.tvoc + ' ppb'           : '-- ppb';
          setSensorBadge('badge-temp', data.current.temp, 10, 28, 35);
          setSensorBadge('badge-hum',  data.current.hum,  20, 80, 90);
          setSensorBadge('badge-lux',  data.current.lux,  200, 800, 2000);
          setSensorBadge('badge-soil', data.current.soil, 30, 60, 80);
          setSensorBadge('badge-eco2', data.current.eco2, 400, 1000, 1500);
          setSensorBadge('badge-tvoc', data.current.tvoc, 0, 220, 660);
          updateAlerts(data.current);
        }

        if (data.system) {
          if (data.system.heap_total && data.system.heap_free) {
            let heapUsage = ((data.system.heap_total - data.system.heap_free) / data.system.heap_total) * 100;
            document.getElementById('val-heap').innerText = heapUsage.toFixed(1) + ' %';
          }
          if (data.system.rssi) {
            document.getElementById('val-rssi').innerText = data.system.rssi + ' dBm';
            let qualityEl = document.getElementById('val-rssi-quality');
            qualityEl.className = 'font-bold';
            if (data.system.rssi >= -50) { qualityEl.innerText = '极好'; qualityEl.classList.add('text-quality-excellent'); }
            else if (data.system.rssi >= -70) { qualityEl.innerText = '良好'; qualityEl.classList.add('text-quality-good'); }
            else if (data.system.rssi >= -80) { qualityEl.innerText = '一般'; qualityEl.classList.add('text-quality-fair'); }
            else { qualityEl.innerText = '较差'; qualityEl.classList.add('text-quality-poor'); }
          }
          if (data.system.chip_rev)  document.getElementById('val-chip-rev').innerText  = 'Rev ' + data.system.chip_rev;
          if (data.system.mac)       document.getElementById('val-mac').innerText        = data.system.mac;
          if (data.system.ip)        document.getElementById('val-ip').innerText         = data.system.ip;
          if (data.system.cpu_freq)  document.getElementById('val-cpu-freq').innerText   = data.system.cpu_freq + ' MHz';
          if (data.system.cpu_cores) document.getElementById('val-cpu-cores').innerText  = data.system.cpu_cores + ' 核';
          if (data.system.uptime !== undefined) document.getElementById('val-uptime').innerText = formatUptime(data.system.uptime);
          if (data.system.sdk_ver)   document.getElementById('val-sdk').innerText        = data.system.sdk_ver;
        }

        // 传感器在线状态
        if (data.sensors) {
          var updateDot = function(id, online) {
            var el = document.getElementById(id);
            el.className = 'status-dot ' + (online ? 'status-connected' : 'status-disconnected');
          };
          updateDot('dot-sht40', data.sensors.sht40);
          updateDot('dot-bh1750', data.sensors.bh1750);
          updateDot('dot-sgp30', data.sensors.sgp30);
          updateDot('dot-soil', data.sensors.soil);
        }

        if (data.history && data.history.time && data.history.time.length > 0) {
          chartTH.setOption({ xAxis: { data: data.history.time }, series: [{ data: data.history.temp || [] }, { data: data.history.hum  || [] }] });
          chartLS.setOption({ xAxis: { data: data.history.time }, series: [{ data: data.history.lux  || [] }, { data: data.history.soil || [] }] });
          chartAQ.setOption({ xAxis: { data: data.history.time }, series: [{ data: data.history.eco2 || [] }, { data: data.history.tvoc || [] }] });
          updateStats(data.history, data.current);
        }

        if (data.state) {
          var isAuto = (data.state.mode === 'auto');
          document.getElementById('cb-mode').checked = isAuto;
          var modeText = document.getElementById('mode-text');
          modeText.innerText = isAuto ? '自动' : '手动';
          modeText.className = isAuto ? 'ml-3 font-bold text-lg w-28 text-left text-green-600' : 'ml-3 font-bold text-lg w-28 text-left text-orange-500';
          ['pump', 'light', 'heater', 'fan'].forEach(function(dev) {
            var cb = document.getElementById('cb-' + dev);
            if (cb) { cb.disabled = isAuto; cb.checked = (data.state[dev] === 1); }
          });
        }

        // 同步调度配置到前端
        if (data.sched) {
          document.getElementById('sched-fan-en').checked = data.sched.fan_en;
          document.getElementById('sched-fan-start').value = data.sched.fan_start || '00:00';
          document.getElementById('sched-fan-end').value = data.sched.fan_end || '00:00';
          document.getElementById('sched-light-en').checked = data.sched.light_en;
          document.getElementById('sched-light-start').value = data.sched.light_start || '00:00';
          document.getElementById('sched-light-end').value = data.sched.light_end || '00:00';
        }

        // 同步系统时间显示
        if (data.time) {
          document.getElementById('val-time-sync').innerText = '系统时间: ' + data.time;
        }

        // 存储信息
        if (data.system && data.system.fs_total) {
          var fsPct = ((data.system.fs_used / data.system.fs_total) * 100).toFixed(1);
          var fsInfo = document.getElementById('storage-info');
          if (fsInfo) fsInfo.innerText = '存储: ' + (data.system.fs_used/1024).toFixed(1) + ' KB / ' + (data.system.fs_total/1024).toFixed(0) + ' KB (' + fsPct + '%)';
        }
      }

      window.toggleMode = function() {
        var isAuto = document.getElementById('cb-mode').checked;
        if (socket && socket.readyState === WebSocket.OPEN) socket.send(JSON.stringify({ action: 'set_mode', mode: isAuto ? 'auto' : 'manual' }));
      };
      window.toggleDevice = function(dev) {
        var isOn = document.getElementById('cb-' + dev).checked;
        if (socket && socket.readyState === WebSocket.OPEN) socket.send(JSON.stringify({ action: 'set_device', device: dev, state: isOn ? 1 : 0 }));
      };

      // ----- 历史数据归档查询 -----
      var histChartTH = null, histChartLS = null, histChartAQ = null;

      function initHistCharts() {
        if (histChartTH) return;
        histChartTH = echarts.init(document.getElementById('hist-chart-th'));
        histChartLS = echarts.init(document.getElementById('hist-chart-ls'));
        histChartAQ = echarts.init(document.getElementById('hist-chart-aq'));
      }

      // 加载可用日期列表
      function loadAvailableDates() {
        fetch('/api/dates').then(function(r) { return r.json(); }).then(function(data) {
          var sel = document.getElementById('hist-date-select');
          sel.innerHTML = '';
          var dates = (data.data_dates || []).sort().reverse();
          if (dates.length === 0) {
            sel.innerHTML = '<option value="">暂无归档数据</option>';
          } else {
            dates.forEach(function(d) {
              var opt = document.createElement('option');
              opt.value = d;
              opt.text = d.substring(0,4) + '-' + d.substring(4,6) + '-' + d.substring(6,8);
              sel.appendChild(opt);
            });
          }
          if (data.total_bytes) {
            var usedPct = ((data.used_bytes / data.total_bytes) * 100).toFixed(1);
            var usedKB = (data.used_bytes / 1024).toFixed(1);
            var totalKB = (data.total_bytes / 1024).toFixed(0);
            document.getElementById('storage-info').innerText =
              '存储: ' + usedKB + ' KB / ' + totalKB + ' KB (' + usedPct + '%)  |  归档天数: ' + dates.length;
          }
        }).catch(function() {
          document.getElementById('storage-info').innerText = '存储状态获取失败';
        });
      }
      // 页面加载后获取日期列表
      setTimeout(loadAvailableDates, 2000);

      window.loadHistoryData = function() {
        var date = document.getElementById('hist-date-select').value;
        if (!date) return;
        fetch('/api/history?date=' + date).then(function(r) { return r.json(); }).then(function(data) {
          if (data.error) { alert(data.error); return; }
          document.getElementById('hist-charts-wrap').style.display = '';
          document.getElementById('hist-stats-wrap').style.display = '';
          document.getElementById('hist-log-wrap').style.display = 'none';
          initHistCharts();

          var dateLabel = date.substring(0,4)+'-'+date.substring(4,6)+'-'+date.substring(6,8);
          histChartTH.setOption({
            title: { text: dateLabel + ' 温湿度', left: 'center', textStyle: { color: '#475569', fontSize: 13 } },
            tooltip: { trigger: 'axis' }, legend: { data: ['温度(°C)','湿度(%)'], top: 22, itemWidth: 10, itemHeight: 10 },
            grid: { left: '15%', right: '15%', bottom: '10%', top: '25%' },
            xAxis: { type: 'category', data: data.time || [] },
            yAxis: [ { type:'value', name:'°C' }, { type:'value', name:'%', position:'right', min:0, max:100 } ],
            series: [
              { name:'温度(°C)', type:'line', data: data.temp||[], smooth:true, itemStyle:{color:'#ef4444'}, areaStyle:{opacity:0.1} },
              { name:'湿度(%)', type:'line', yAxisIndex:1, data: data.hum||[], smooth:true, itemStyle:{color:'#3b82f6'}, areaStyle:{opacity:0.1} }
            ]
          });
          histChartLS.setOption({
            title: { text: dateLabel + ' 光照与土壤', left: 'center', textStyle: { color: '#475569', fontSize: 13 } },
            tooltip: { trigger: 'axis' }, legend: { data: ['光照(lx)','土壤(%)'], top: 22, itemWidth: 10, itemHeight: 10 },
            grid: { left: '15%', right: '15%', bottom: '10%', top: '25%' },
            xAxis: { type: 'category', data: data.time || [] },
            yAxis: [ { type:'value', name:'lx' }, { type:'value', name:'%', position:'right', min:0, max:100 } ],
            series: [
              { name:'光照(lx)', type:'line', data: data.lux||[], smooth:true, itemStyle:{color:'#f97316'}, areaStyle:{opacity:0.1} },
              { name:'土壤(%)', type:'line', yAxisIndex:1, data: data.soil||[], smooth:true, itemStyle:{color:'#22c55e'}, areaStyle:{opacity:0.1} }
            ]
          });
          histChartAQ.setOption({
            title: { text: dateLabel + ' 空气质量', left: 'center', textStyle: { color: '#475569', fontSize: 13 } },
            tooltip: { trigger: 'axis' }, legend: { data: ['eCO2(ppm)','TVOC(ppb)'], top: 22, itemWidth: 10, itemHeight: 10 },
            grid: { left: '15%', right: '15%', bottom: '10%', top: '25%' },
            xAxis: { type: 'category', data: data.time || [] },
            yAxis: [ { type:'value', name:'ppm', min:400 }, { type:'value', name:'ppb', position:'right' } ],
            series: [
              { name:'eCO2(ppm)', type:'line', data: data.eco2||[], smooth:true, itemStyle:{color:'#14b8a6'}, areaStyle:{opacity:0.1} },
              { name:'TVOC(ppb)', type:'line', yAxisIndex:1, data: data.tvoc||[], smooth:true, itemStyle:{color:'#a855f7'}, areaStyle:{opacity:0.1} }
            ]
          });

          // 统计摘要
          var metrics = [
            {key:'temp', label:'🌡️ 温度', unit:'°C', dec:1},
            {key:'hum', label:'💧 湿度', unit:'%', dec:1},
            {key:'lux', label:'☀️ 光照', unit:'lx', dec:0},
            {key:'soil', label:'🌱 土壤', unit:'%', dec:0},
            {key:'eco2', label:'☁️ eCO2', unit:'ppm', dec:0},
            {key:'tvoc', label:'🧪 TVOC', unit:'ppb', dec:0}
          ];
          var rows = '';
          metrics.forEach(function(m) {
            var s = calcStats(data[m.key]);
            if (s) {
              rows += '<tr class="border-b border-slate-100 hover:bg-slate-50">';
              rows += '<td class="px-3 py-2 font-medium text-gray-700 text-left">' + m.label + '</td>';
              rows += '<td class="px-3 py-2 text-blue-600 font-mono">' + s.min.toFixed(m.dec) + ' ' + m.unit + '</td>';
              rows += '<td class="px-3 py-2 text-red-500 font-mono">' + s.max.toFixed(m.dec) + ' ' + m.unit + '</td>';
              rows += '<td class="px-3 py-2 text-gray-600 font-mono">' + s.avg.toFixed(m.dec) + ' ' + m.unit + '</td>';
              rows += '<td class="px-3 py-2 text-green-700 font-mono font-bold">' + (data[m.key]||[]).length + '</td>';
              rows += '</tr>';
            }
          });
          document.getElementById('hist-stats-body').innerHTML = rows;
        }).catch(function(e) { alert('查询失败: ' + e.message); });
      };

      window.loadHistoryLog = function() {
        var date = document.getElementById('hist-date-select').value;
        if (!date) return;
        fetch('/api/log?date=' + date).then(function(r) { return r.json(); }).then(function(data) {
          if (data.error) { alert(data.error); return; }
          document.getElementById('hist-charts-wrap').style.display = 'none';
          document.getElementById('hist-stats-wrap').style.display = 'none';
          document.getElementById('hist-log-wrap').style.display = '';
          var list = document.getElementById('hist-log-list');
          if (!data.entries || data.entries.length === 0) {
            list.innerHTML = '<div class="text-gray-400">当日无操作记录</div>';
          } else {
            list.innerHTML = data.entries.map(function(e) { return '<div class="py-1 border-b border-slate-200">' + e.replace(/</g,'&lt;') + '</div>'; }).join('');
          }
        }).catch(function(e) { alert('查询失败: ' + e.message); });
      };

      // ----- 定时调度保存与同步 -----
      window.saveSchedule = function() {
        if (!socket || socket.readyState !== WebSocket.OPEN) return;
        socket.send(JSON.stringify({
          action: 'set_sched',
          fan_en: document.getElementById('sched-fan-en').checked,
          fan_start: document.getElementById('sched-fan-start').value || '00:00',
          fan_end: document.getElementById('sched-fan-end').value || '00:00',
          light_en: document.getElementById('sched-light-en').checked,
          light_start: document.getElementById('sched-light-start').value || '00:00',
          light_end: document.getElementById('sched-light-end').value || '00:00'
        }));
      };

      // ----- QQ Bot 配置 -----
      function loadQQBotConfig() {
        fetch('/api/qqbot/config').then(function(r) { return r.json(); }).then(function(data) {
          document.getElementById('qqbot-appid').value = data.appId || '';
          document.getElementById('qqbot-groupid').value = data.userOpenId || '';
          document.getElementById('qqbot-enabled').checked = data.enabled || false;
          var status = document.getElementById('qqbot-status');
          if (!data.appId) { status.innerText = '未配置'; status.className = 'text-xs font-medium text-gray-400'; }
          else if (data.gatewayReady) { status.innerText = 'Gateway 就绪'; status.className = 'text-xs font-medium text-green-600'; }
          else if (data.tokenValid) { status.innerText = 'Token 有效'; status.className = 'text-xs font-medium text-blue-600'; }
          else if (data.enabled) { status.innerText = '已启用'; status.className = 'text-xs font-medium text-blue-600'; }
          else { status.innerText = '已禁用'; status.className = 'text-xs font-medium text-orange-500'; }
          // Gateway 状态指示
          var gwDot = document.getElementById('gw-status-dot');
          var gwText = document.getElementById('gw-status-text');
          if (data.gatewayReady) {
            gwDot.style.background = '#22c55e'; gwText.innerText = '已连接，可接收消息';
          } else if (data.gatewayConnected) {
            gwDot.style.background = '#f59e0b'; gwText.innerText = '连接中...';
          } else if (data.enabled) {
            gwDot.style.background = '#ef4444'; gwText.innerText = '未连接，等待重连';
          } else {
            gwDot.style.background = '#9ca3af'; gwText.innerText = '未启用';
          }
        }).catch(function() {});
      }
      setTimeout(loadQQBotConfig, 3000);
      setInterval(loadQQBotConfig, 15000);  // 定期刷新 Gateway 状态

      window.saveQQBotConfig = function() {
        var secret = document.getElementById('qqbot-secret').value;
        var body = {
          appId: document.getElementById('qqbot-appid').value,
          userOpenId: document.getElementById('qqbot-groupid').value,
          enabled: document.getElementById('qqbot-enabled').checked
        };
        if (secret) body.appSecret = secret;
        fetch('/api/qqbot/config', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(body)
        }).then(function(r) { return r.json(); }).then(function(data) {
          if (data.ok) { alert('QQ Bot 配置已保存'); loadQQBotConfig(); }
        }).catch(function(e) { alert('保存失败: ' + e.message); });
      };

      window.testQQBot = function() {
        fetch('/api/qqbot/test').then(function(r) { return r.json(); }).then(function(data) {
          alert(data.ok ? '测试消息发送成功！' : ('发送失败: ' + (data.error || '未知错误')));
        }).catch(function(e) { alert('请求失败: ' + e.message); });
      };

      // ----- AI 助手交互逻辑 -----
      function appendChat(role, msg) {
        const box = document.getElementById('ai-chat-box');
        const outer = document.createElement('div');
        outer.className = role === 'user' ? 'flex justify-end pr-2 pl-8' : 'flex justify-start pl-2 pr-8';
        const inner = document.createElement('div');
        inner.className = role === 'user' 
          ? 'bg-blue-600 text-white p-3 rounded-2xl rounded-tr-none text-sm break-words' 
          : 'bg-white border border-gray-100 shadow-sm p-3 rounded-2xl rounded-tl-none text-sm text-gray-700 leading-relaxed break-words whitespace-pre-wrap';
        inner.innerText = msg;
        outer.appendChild(inner);
        box.appendChild(outer);
        box.scrollTop = box.scrollHeight;
      }

      window.askAI = function() {
        if (socket && socket.readyState === WebSocket.OPEN) {
          const btn = document.getElementById('btn-ai-analyze');
          btn.disabled = true;
          btn.innerText = '分析中...';
          btn.classList.add('opacity-50', 'cursor-not-allowed');
          appendChat('user', '请根据当前环境数据（温湿度、CO2、光照）分析大棚状态并给出农业建议。');
          socket.send(JSON.stringify({ action: 'ai_analyze' }));
        }
      };

      window.sendAIChat = function() {
        const input = document.getElementById('ai-input');
        const q = input.value.trim();
        if (!q || !socket || socket.readyState !== WebSocket.OPEN) return;
        appendChat('user', q);
        socket.send(JSON.stringify({ action: 'ai_chat', question: q }));
        input.value = '';
      };

      document.getElementById('ai-input') && document.getElementById('ai-input').addEventListener('keypress', function(e) {
        if (e.key === 'Enter') sendAIChat();
      });

      // ----- 扩展 updateUI 处理 AI 响应 -----
      const originalUpdateUI = updateUI;
      updateUI = function(data) {
        if (data.ai_resp) {
          const btn = document.getElementById('btn-ai-analyze');
          btn.disabled = false;
          btn.innerText = '实时环境分析';
          btn.classList.remove('opacity-50', 'cursor-not-allowed');
          appendChat('ai', data.ai_resp);
        }
        originalUpdateUI(data);
      };

      window.addEventListener('resize', function() { chartTH.resize(); chartLS.resize(); chartAQ.resize(); });
    })();
  </script>
</body>
</html>
)rawliteral";