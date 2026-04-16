#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>AI-Biodome 鏅烘収澶ф</title>
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
    <div class="logo">馃尡</div>
    <div>
      <div class="brand-text">AI-Biodome</div>
      <div class="brand-sub">鏅烘収澶ф鎺у埗鍙?/div>
    </div>
  </div>
  <div class="nav-list">
    <button class="nav-item active" onclick="switchTab('dashboard')" data-tab="dashboard">
      <span class="icon">馃搳</span><span class="nav-label">浠〃鐩?/span>
    </button>
    <button class="nav-item" onclick="switchTab('control')" data-tab="control">
      <span class="icon">鈿欙笍</span><span class="nav-label">璁惧鎺у埗</span>
    </button>
    <button class="nav-item" onclick="switchTab('history')" data-tab="history">
      <span class="icon">馃搧</span><span class="nav-label">鍘嗗彶鏁版嵁</span>
    </button>
    <button class="nav-item" onclick="switchTab('qqbot')" data-tab="qqbot">
      <span class="icon">馃</span><span class="nav-label">QQ 鏈哄櫒浜?/span>
    </button>
    <button class="nav-item" onclick="switchTab('system')" data-tab="system">
      <span class="icon">馃搵</span><span class="nav-label">绯荤粺淇℃伅</span>
    </button>
  </div>
  <div class="sidebar-footer">
    <div class="conn-badge">
      <span class="conn-dot off" id="ws-dot"></span>
      <span id="ws-status-text">鏈繛鎺?/span>
    </div>
    <div class="conn-badge" style="margin-top:4px">
      <span class="conn-dot off" id="gw-dot-side"></span>
      <span id="gw-status-side">Gateway</span>
    </div>
  </div>
</nav>

<!-- Top Bar (mobile) -->
<div class="topbar" id="topbar">
  <button class="menu-btn" onclick="openSidebar()">鈽?/button>
  <span class="font-bold text-sm text-gray-700">馃尡 AI-Biodome</span>
  <div class="flex items-center gap-2">
    <span class="conn-dot off" id="ws-dot-mobile" style="width:8px;height:8px;border-radius:50%"></span>
  </div>
</div>

<!-- Bottom Tabs (mobile) -->
<div class="bottom-tabs" id="bottom-tabs">
  <button class="tab-btn active" onclick="switchTab('dashboard')" data-tab="dashboard"><span class="t-icon">馃搳</span>浠〃鐩?/button>
  <button class="tab-btn" onclick="switchTab('control')" data-tab="control"><span class="t-icon">鈿欙笍</span>鎺у埗</button>
  <button class="tab-btn" onclick="switchTab('history')" data-tab="history"><span class="t-icon">馃搧</span>鍘嗗彶</button>
  <button class="tab-btn" onclick="switchTab('qqbot')" data-tab="qqbot"><span class="t-icon">馃</span>鏈哄櫒浜?/button>
  <button class="tab-btn" onclick="switchTab('system')" data-tab="system"><span class="t-icon">馃搵</span>绯荤粺</button>
</div>

<!-- AI Chat Float Button -->
<button class="ai-float-btn" id="ai-float-btn" onclick="toggleAIChat()">馃挰</button>

<!-- AI Chat Drawer -->
<div class="ai-drawer" id="ai-drawer">
  <div class="ai-drawer-head">
    <div class="flex items-center gap-2"><span class="text-lg">馃</span><span class="font-bold text-sm">AI 鏅鸿兘鍔╂墜</span></div>
    <div class="flex items-center gap-2">
      <button id="btn-ai-analyze" onclick="askAI()" class="px-3 py-1 bg-white/20 hover:bg-white/30 rounded-lg text-xs font-medium transition-colors">鐜鍒嗘瀽</button>
      <button onclick="toggleAIChat()" class="text-white/80 hover:text-white text-lg" style="background:none;border:none;cursor:pointer">鉁?/button>
    </div>
  </div>
  <div class="ai-drawer-body" id="ai-chat-box">
    <div class="flex justify-start pl-2 pr-8">
      <div class="bg-gray-50 border border-gray-100 p-3 rounded-2xl rounded-tl-none text-sm text-gray-600">浣犲ソ锛佹垜鏄?AI 鍔╂墜锛屽彲浠ュ府浣犲垎鏋愬ぇ妫氱幆澧冩暟鎹€傜偣鍑?鐜鍒嗘瀽"鎴栫洿鎺ユ彁闂€?/div>
    </div>
  </div>
  <div class="ai-drawer-foot">
    <input type="text" id="ai-input" placeholder="杈撳叆浣犵殑闂..." class="flex-1 px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
    <button onclick="sendAIChat()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">鍙戦€?/button>
  </div>
</div>

<!-- Main Content -->
<div class="main-wrap" id="main-wrap">

<!-- ===== Dashboard Panel ===== -->
<div class="panel active" id="panel-dashboard">
  <div class="flex items-center justify-between mb-5">
    <div>
      <h1 class="text-xl font-bold text-gray-800">瀹炴椂鐩戞帶</h1>
      <p class="text-xs text-gray-400 mt-1" id="val-time-sync">绯荤粺鏃堕棿: --</p>
    </div>
    <div class="flex items-center gap-3">
      <span class="text-xs px-3 py-1 rounded-full bg-blue-50 text-blue-600 font-medium" id="mode-badge">鑷姩妯″紡</span>
    </div>
  </div>

  <!-- Sensor Cards -->
  <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4 mb-6">
    <div class="card card-sm sensor-card sc-temp">
      <div class="flex items-center justify-between"><span class="sensor-icon">馃尅锔?/span><span class="s-badge" id="badge-temp">--</span></div>
      <div class="sensor-val" id="val-temp">-- 掳C</div>
      <div class="sensor-label">娓╁害</div>
    </div>
    <div class="card card-sm sensor-card sc-hum">
      <div class="flex items-center justify-between"><span class="sensor-icon">馃挧</span><span class="s-badge" id="badge-hum">--</span></div>
      <div class="sensor-val" id="val-hum">-- %</div>
      <div class="sensor-label">婀垮害</div>
    </div>
    <div class="card card-sm sensor-card sc-lux">
      <div class="flex items-center justify-between"><span class="sensor-icon">鈽€锔?/span><span class="s-badge" id="badge-lux">--</span></div>
      <div class="sensor-val" id="val-lux">-- lx</div>
      <div class="sensor-label">鍏夌収</div>
    </div>
    <div class="card card-sm sensor-card sc-soil">
      <div class="flex items-center justify-between"><span class="sensor-icon">馃尡</span><span class="s-badge" id="badge-soil">--</span></div>
      <div class="sensor-val" id="val-soil">-- %</div>
      <div class="sensor-label">鍦熷￥姘村垎</div>
    </div>
    <div class="card card-sm sensor-card sc-co2">
      <div class="flex items-center justify-between"><span class="sensor-icon">鈽侊笍</span><span class="s-badge" id="badge-eco2">--</span></div>
      <div class="sensor-val" id="val-eco2">-- ppm</div>
      <div class="sensor-label">eCO2</div>
    </div>
    <div class="card card-sm sensor-card sc-tvoc">
      <div class="flex items-center justify-between"><span class="sensor-icon">馃И</span><span class="s-badge" id="badge-tvoc">--</span></div>
      <div class="sensor-val" id="val-tvoc">-- ppb</div>
      <div class="sensor-label">TVOC</div>
    </div>
  </div>

  <!-- Alert Panel -->
  <div class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">鈿?鐜棰勮</h3>
    <div id="alert-list"><div class="text-green-600 text-sm font-medium">鉁?鐜鐘舵€佽壇濂?/div></div>
  </div>

  <!-- Charts -->
  <div class="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
    <div class="card card-sm"><div class="chart-box" id="chart-th"></div></div>
    <div class="card card-sm"><div class="chart-box" id="chart-ls"></div></div>
    <div class="card card-sm"><div class="chart-box" id="chart-aq"></div></div>
  </div>

  <!-- Statistics Table -->
  <div class="card">
    <h3 class="text-sm font-bold text-gray-600 mb-3">馃搱 瀹炴椂缁熻</h3>
    <div class="overflow-x-auto">
      <table class="w-full text-sm text-center">
        <thead><tr class="bg-slate-50 text-gray-500 text-xs">
          <th class="px-3 py-2 text-left font-semibold">鎸囨爣</th>
          <th class="px-3 py-2 font-semibold">鏈€灏?/th>
          <th class="px-3 py-2 font-semibold">鏈€澶?/th>
          <th class="px-3 py-2 font-semibold">骞冲潎</th>
          <th class="px-3 py-2 font-semibold">褰撳墠</th>
        </tr></thead>
        <tbody id="stats-body"></tbody>
      </table>
    </div>
  </div>
</div>

<!-- ===== Control Panel ===== -->
<div class="panel" id="panel-control">
  <h1 class="text-xl font-bold text-gray-800 mb-5">璁惧鎺у埗</h1>

  <!-- Mode Switch -->
  <div class="card mb-6">
    <div class="flex items-center justify-between">
      <div>
        <h3 class="font-bold text-gray-800">杩愯妯″紡</h3>
        <p class="text-xs text-gray-400 mt-1">鑷姩妯″紡涓嬭澶囨牴鎹紶鎰熷櫒鏁版嵁鑷姩璋冭妭</p>
      </div>
      <div class="flex items-center gap-3">
        <span class="text-sm font-bold" id="mode-text" style="color:#16a34a">鑷姩</span>
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
        <div class="dev-icon bg-blue-50"><span>馃挧</span></div>
        <div><div class="font-bold text-gray-800 text-sm">姘存车</div><div class="text-xs text-gray-400">鍦熷￥鐏屾簤</div></div>
      </div>
      <div class="toggle disabled" id="cb-pump" onclick="toggleDevice('pump')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-yellow-50"><span>馃挕</span></div>
        <div><div class="font-bold text-gray-800 text-sm">琛ュ厜鐏?/div><div class="text-xs text-gray-400">妞嶇墿琛ュ厜</div></div>
      </div>
      <div class="toggle disabled" id="cb-light" onclick="toggleDevice('light')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-red-50"><span>馃敟</span></div>
        <div><div class="font-bold text-gray-800 text-sm">鍔犵儹鍨?/div><div class="text-xs text-gray-400">娓╁害璋冭妭</div></div>
      </div>
      <div class="toggle disabled" id="cb-heater" onclick="toggleDevice('heater')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-cyan-50"><span>馃寑</span></div>
        <div><div class="font-bold text-gray-800 text-sm">鎺掗鎵?/div><div class="text-xs text-gray-400">閫氶鎹㈡皵</div></div>
      </div>
      <div class="toggle disabled" id="cb-fan" onclick="toggleDevice('fan')"></div>
    </div>
  </div>

  <!-- Schedule -->
  <div class="card">
    <h3 class="font-bold text-gray-800 mb-4">鈴?瀹氭椂浠诲姟</h3>
    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
      <div class="bg-slate-50 rounded-xl p-4">
        <div class="flex items-center justify-between mb-3">
          <span class="font-semibold text-sm text-gray-700">馃寑 瀹氭椂閫氶</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="sched-fan-en" class="sr-only peer" onchange="saveSchedule()">
            <div class="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
        <div class="flex items-center gap-2 text-sm">
          <input type="time" id="sched-fan-start" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
          <span class="text-gray-400">鑷?/span>
          <input type="time" id="sched-fan-end" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
        </div>
      </div>
      <div class="bg-slate-50 rounded-xl p-4">
        <div class="flex items-center justify-between mb-3">
          <span class="font-semibold text-sm text-gray-700">馃挕 瀹氭椂琛ュ厜</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="sched-light-en" class="sr-only peer" onchange="saveSchedule()">
            <div class="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
        <div class="flex items-center gap-2 text-sm">
          <input type="time" id="sched-light-start" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
          <span class="text-gray-400">鑷?/span>
          <input type="time" id="sched-light-end" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
        </div>
      </div>
    </div>
  </div>
</div>

<!-- ===== History Panel ===== -->
<div class="panel" id="panel-history">
  <h1 class="text-xl font-bold text-gray-800 mb-5">鍘嗗彶鏁版嵁</h1>

  <div class="card mb-6">
    <div class="flex flex-wrap items-center gap-3 mb-3">
      <select id="hist-date-select" class="px-3 py-2 rounded-lg border text-sm focus:ring-2 focus:ring-blue-400 outline-none">
        <option value="">鍔犺浇涓?..</option>
      </select>
      <button onclick="loadHistoryData()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">鏌ヨ鏁版嵁</button>
      <button onclick="loadHistoryLog()" class="px-4 py-2 bg-gray-500 hover:bg-gray-600 text-white rounded-lg text-sm font-medium transition-colors">鏌ョ湅鏃ュ織</button>
    </div>
    <div class="text-xs text-gray-400" id="storage-info">瀛樺偍鐘舵€佸姞杞戒腑...</div>
  </div>

  <!-- History Charts -->
  <div id="hist-charts-wrap" style="display:none" class="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
    <div class="card card-sm"><div class="chart-box" id="hist-chart-th"></div></div>
    <div class="card card-sm"><div class="chart-box" id="hist-chart-ls"></div></div>
    <div class="card card-sm"><div class="chart-box" id="hist-chart-aq"></div></div>
  </div>

  <!-- History Stats -->
  <div id="hist-stats-wrap" style="display:none" class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">褰撴棩缁熻鎽樿</h3>
    <div class="overflow-x-auto">
      <table class="w-full text-sm text-center">
        <thead><tr class="bg-slate-50 text-gray-500 text-xs">
          <th class="px-3 py-2 text-left font-semibold">鎸囨爣</th>
          <th class="px-3 py-2 font-semibold">鏈€灏?/th>
          <th class="px-3 py-2 font-semibold">鏈€澶?/th>
          <th class="px-3 py-2 font-semibold">骞冲潎</th>
          <th class="px-3 py-2 font-semibold">鏁版嵁鐐?/th>
        </tr></thead>
        <tbody id="hist-stats-body"></tbody>
      </table>
    </div>
  </div>

  <!-- Log Viewer -->
  <div id="hist-log-wrap" style="display:none" class="card">
    <h3 class="text-sm font-bold text-gray-600 mb-3">鎿嶄綔鏃ュ織</h3>
    <div id="hist-log-list" class="bg-slate-50 rounded-lg p-4 max-h-80 overflow-y-auto text-xs font-mono text-gray-700 space-y-1"></div>
  </div>
</div>

<!-- ===== QQ Bot Panel ===== -->
<div class="panel" id="panel-qqbot">
  <h1 class="text-xl font-bold text-gray-800 mb-5">QQ 鏈哄櫒浜?/h1>

  <div class="card mb-6">
    <div class="flex items-center justify-between mb-4">
      <div class="flex items-center gap-3">
        <span class="text-2xl">馃</span>
        <div>
          <h3 class="font-bold text-gray-800">鏈哄櫒浜洪厤缃?/h3>
          <span class="text-xs font-medium" id="qqbot-status">鏈厤缃?/span>
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
        <label class="block text-xs font-semibold text-gray-500 mb-1">鐢ㄦ埛 OpenID锛堝彲閫夛級</label>
        <input type="text" id="qqbot-groupid" placeholder="涓诲姩鎺ㄩ€佺敤" class="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
      </div>
    </div>
    <div class="flex flex-wrap items-center gap-3">
      <button onclick="saveQQBotConfig()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">淇濆瓨閰嶇疆</button>
      <button onclick="testQQBot()" class="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg text-sm font-medium transition-colors">鍙戦€佹祴璇?/button>
    </div>
  </div>

  <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
    <div class="card">
      <h3 class="text-sm font-bold text-gray-600 mb-3">Gateway 鐘舵€?/h3>
      <div class="flex items-center gap-3 mb-3">
        <span class="w-3 h-3 rounded-full" id="gw-status-dot" style="background:#9ca3af"></span>
        <span class="text-sm font-medium" id="gw-status-text">鏈繛鎺?/span>
      </div>
      <p class="text-xs text-gray-400">Gateway 杩炴帴鍚庯紝鐢ㄦ埛鍙鑱婃満鍣ㄤ汉鏌ヨ澶ф鐘舵€?/p>
    </div>
    <div class="card">
      <h3 class="text-sm font-bold text-gray-600 mb-3">鏀寔鐨勬寚浠?/h3>
      <div class="space-y-2">
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">鐘舵€?/code><span class="text-gray-600">鏌ョ湅鐜鏁版嵁</span></div>
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">鍛婅</code><span class="text-gray-600">鏌ョ湅褰撳墠鍛婅</span></div>
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">甯姪</code><span class="text-gray-600">鏄剧ず鎸囦护鍒楄〃</span></div>
      </div>
    </div>
  </div>
</div>

<!-- ===== System Panel ===== -->
<div class="panel" id="panel-system">
  <h1 class="text-xl font-bold text-gray-800 mb-5">绯荤粺淇℃伅</h1>

  <!-- Sensor Status -->
  <div class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">浼犳劅鍣ㄧ姸鎬?/h3>
    <div class="grid grid-cols-2 md:grid-cols-4 gap-3">
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-sht40"></span>
        <span class="text-sm text-gray-600">SHT40 娓╂箍搴?/span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-bh1750"></span>
        <span class="text-sm text-gray-600">BH1750 鍏夌収</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-sgp30"></span>
        <span class="text-sm text-gray-600">SGP30 绌烘皵</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot on" id="dot-soil"></span>
        <span class="text-sm text-gray-600">ADC 鍦熷￥</span>
      </div>
    </div>
  </div>

  <!-- System Info -->
  <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">鍐呭瓨浣跨敤</div>
      <div class="text-lg font-bold text-gray-800" id="val-heap">-- %</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">WiFi 淇″彿</div>
      <div class="text-lg font-bold text-gray-800"><span id="val-rssi">--</span> <span class="text-sm font-normal" id="val-rssi-quality"></span></div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">杩愯鏃堕棿</div>
      <div class="text-lg font-bold text-gray-800" id="val-uptime">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">鑺墖淇℃伅</div>
      <div class="text-sm font-bold text-gray-800"><span id="val-chip-rev">--</span> 路 <span id="val-cpu-cores">--</span> 路 <span id="val-cpu-freq">--</span></div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">缃戠粶</div>
      <div class="text-sm font-bold text-gray-800" id="val-ip">--</div>
      <div class="text-xs text-gray-400 mt-1" id="val-mac">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">SDK 鐗堟湰</div>
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
  chartTH.setOption(makeChartOpt('娓╂箍搴?,['娓╁害(掳C)','婀垮害(%)'],['#ef4444','#3b82f6'],['掳C','%']));
  chartLS.setOption(makeChartOpt('鍏夌収路鍦熷￥',['鍏夌収(lx)','鍦熷￥(%)'],['#f97316','#22c55e'],['lx','%']));
  chartAQ.setOption(makeChartOpt('绌烘皵璐ㄩ噺',['eCO2(ppm)','TVOC(ppb)'],['#14b8a6','#a855f7'],['ppm','ppb'],400));

  // ===== WebSocket =====
  var gateway = 'ws://' + window.location.hostname + ':81/';
  var socket = null;
  var reconnectTimer = null;

  function setWsStatus(s) {
    var dots = [document.getElementById('ws-dot'), document.getElementById('ws-dot-mobile')];
    var text = document.getElementById('ws-status-text');
    dots.forEach(function(d){ if(!d)return; d.className='conn-dot '+(s==='on'?'on':s==='pending'?'pending':'off'); });
    if(text) text.innerText = s==='on'?'宸茶繛鎺?:s==='pending'?'杩炴帴涓?:'鏈繛鎺?;
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
    if(val<low){el.className='s-badge b-low';el.innerText='鍋忎綆';}
    else if(val>high){el.className='s-badge b-high';el.innerText='鍋忛珮';}
    else if(val>warnH){el.className='s-badge b-warn';el.innerText='娉ㄦ剰';}
    else{el.className='s-badge b-ok';el.innerText='姝ｅ父';}
  }

  function fmtUptime(s){
    var d=Math.floor(s/86400),h=Math.floor((s%86400)/3600),m=Math.floor((s%3600)/60),sec=s%60;
    if(d>0)return d+'澶?'+h+'鏃?'+m+'鍒?;
    if(h>0)return h+'鏃?'+m+'鍒?'+sec+'绉?;
    return m+'鍒?'+sec+'绉?;
  }

  function calcStats(arr){
    if(!arr||arr.length===0)return null;
    var min=Infinity,max=-Infinity,sum=0;
    for(var i=0;i<arr.length;i++){var v=parseFloat(arr[i]);if(v<min)min=v;if(v>max)max=v;sum+=v;}
    return{min:min,max:max,avg:sum/arr.length};
  }

  function buildStatsRows(history, current, tbodyId) {
    var metrics=[
      {key:'temp',label:'馃尅锔?娓╁害',unit:'掳C',dec:1},
      {key:'hum',label:'馃挧 婀垮害',unit:'%',dec:1},
      {key:'lux',label:'鈽€锔?鍏夌収',unit:'lx',dec:0},
      {key:'soil',label:'馃尡 鍦熷￥',unit:'%',dec:0},
      {key:'eco2',label:'鈽侊笍 eCO2',unit:'ppm',dec:0},
      {key:'tvoc',label:'馃И TVOC',unit:'ppb',dec:0}
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
      if(c.temp>35)a.push({cls:'alert-danger',msg:'鈿狅笍 娓╁害杩囬珮 ('+c.temp.toFixed(1)+'掳C)'});
      else if(c.temp<10)a.push({cls:'alert-danger',msg:'馃ザ 娓╁害杩囦綆 ('+c.temp.toFixed(1)+'掳C)'});
      else if(c.temp>28)a.push({cls:'alert-warn',msg:'馃尅锔?娓╁害鍋忛珮 ('+c.temp.toFixed(1)+'掳C)'});
      else if(c.temp<15)a.push({cls:'alert-warn',msg:'馃尅锔?娓╁害鍋忎綆 ('+c.temp.toFixed(1)+'掳C)'});
    }
    if(c.hum!=null){
      if(c.hum>90)a.push({cls:'alert-danger',msg:'馃挧 婀垮害杩囬珮 ('+c.hum.toFixed(1)+'%)'});
      else if(c.hum<20)a.push({cls:'alert-warn',msg:'馃挧 婀垮害鍋忎綆 ('+c.hum.toFixed(1)+'%)'});
    }
    if(c.soil!=null){
      if(c.soil<20)a.push({cls:'alert-danger',msg:'馃尡 鍦熷￥涓ラ噸缂烘按 ('+c.soil+'%)'});
      else if(c.soil<30)a.push({cls:'alert-warn',msg:'馃尡 鍦熷￥鍋忓共 ('+c.soil+'%)'});
    }
    if(c.eco2!=null){
      if(c.eco2>1500)a.push({cls:'alert-danger',msg:'鈽侊笍 CO2瓒呮爣 ('+c.eco2+'ppm)'});
      else if(c.eco2>1000)a.push({cls:'alert-warn',msg:'鈽侊笍 CO2鍋忛珮 ('+c.eco2+'ppm)'});
    }
    if(c.tvoc!=null){
      if(c.tvoc>660)a.push({cls:'alert-danger',msg:'馃И TVOC瓒呮爣 ('+c.tvoc+'ppb)'});
      else if(c.tvoc>220)a.push({cls:'alert-warn',msg:'馃И TVOC鍋忛珮 ('+c.tvoc+'ppb)'});
    }
    var el=document.getElementById('alert-list');
    el.innerHTML=a.length===0?'<div class="text-green-600 text-sm font-medium">鉁?鐜鐘舵€佽壇濂?/div>':
      a.map(function(x){return '<div class="alert-item '+x.cls+'">'+x.msg+'</div>';}).join('');
  }

  // ===== Main UI Update =====
  function updateUI(data){
    if(data.ai_resp){
      var btn=document.getElementById('btn-ai-analyze');
      if(btn){btn.disabled=false;btn.innerText='鐜鍒嗘瀽';btn.classList.remove('opacity-50','cursor-not-allowed');}
      appendChat('ai',data.ai_resp);
    }
    if(data.current){
      var c=data.current;
      document.getElementById('val-temp').innerText=(c.temp!=null)?c.temp.toFixed(1)+' 掳C':'-- 掳C';
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
        if(sys.rssi>=-50){q.innerText='鏋佸ソ';q.style.color='#16a34a';}
        else if(sys.rssi>=-70){q.innerText='鑹ソ';q.style.color='#2563eb';}
        else if(sys.rssi>=-80){q.innerText='涓€鑸?;q.style.color='#d97706';}
        else{q.innerText='杈冨樊';q.style.color='#dc2626';}
      }
      if(sys.chip_rev)document.getElementById('val-chip-rev').innerText='Rev '+sys.chip_rev;
      if(sys.mac)document.getElementById('val-mac').innerText=sys.mac;
      if(sys.ip)document.getElementById('val-ip').innerText=sys.ip;
      if(sys.cpu_freq)document.getElementById('val-cpu-freq').innerText=sys.cpu_freq+'MHz';
      if(sys.cpu_cores)document.getElementById('val-cpu-cores').innerText=sys.cpu_cores+'鏍?;
      if(sys.uptime!=null)document.getElementById('val-uptime').innerText=fmtUptime(sys.uptime);
      if(sys.sdk_ver)document.getElementById('val-sdk').innerText=sys.sdk_ver;
      if(sys.fs_total){
        var fsPct=(sys.fs_used/sys.fs_total*100).toFixed(1);
        var si=document.getElementById('storage-info');
        if(si) si.innerText='瀛樺偍: '+(sys.fs_used/1024).toFixed(1)+'KB / '+(sys.fs_total/1024).toFixed(0)+'KB ('+fsPct+'%)';
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
      mt.innerText=isAuto?'鑷姩':'鎵嬪姩';
      mt.style.color=isAuto?'#16a34a':'#ea580c';
      document.getElementById('mode-badge').innerText=isAuto?'鑷姩妯″紡':'鎵嬪姩妯″紡';
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
      document.getElementById('val-time-sync').innerText='绯荤粺鏃堕棿: '+data.time;
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
      btn.innerText = '鍒嗘瀽涓?..';
      btn.classList.add('opacity-50', 'cursor-not-allowed');
      appendChat('user', '璇锋牴鎹綋鍓嶇幆澧冩暟鎹垎鏋愬ぇ妫氱姸鎬佸苟缁欏嚭寤鸿銆?);
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
      if(dates.length===0){sel.innerHTML='<option value="">鏆傛棤褰掓。</option>';}
      else{dates.forEach(function(d){var o=document.createElement('option');o.value=d;o.text=d.substring(0,4)+'-'+d.substring(4,6)+'-'+d.substring(6,8);sel.appendChild(o);});}
      if(data.total_bytes){
        var si=document.getElementById('storage-info');
        if(si) si.innerText='瀛樺偍: '+(data.used_bytes/1024).toFixed(1)+'KB / '+(data.total_bytes/1024).toFixed(0)+'KB | 褰掓。: '+dates.length+'澶?;
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
      histChartTH.setOption(makeChartOpt(dl+' 娓╂箍搴?,['娓╁害(掳C)','婀垮害(%)'],['#ef4444','#3b82f6'],['掳C','%']));
      histChartTH.setOption({xAxis:{data:data.time||[]},series:[{data:data.temp||[]},{data:data.hum||[]}]});
      histChartLS.setOption(makeChartOpt(dl+' 鍏夌収路鍦熷￥',['鍏夌収(lx)','鍦熷￥(%)'],['#f97316','#22c55e'],['lx','%']));
      histChartLS.setOption({xAxis:{data:data.time||[]},series:[{data:data.lux||[]},{data:data.soil||[]}]});
      histChartAQ.setOption(makeChartOpt(dl+' 绌烘皵璐ㄩ噺',['eCO2(ppm)','TVOC(ppb)'],['#14b8a6','#a855f7'],['ppm','ppb'],400));
      histChartAQ.setOption({xAxis:{data:data.time||[]},series:[{data:data.eco2||[]},{data:data.tvoc||[]}]});
      var metrics=[{key:'temp',label:'馃尅锔?娓╁害',unit:'掳C',dec:1},{key:'hum',label:'馃挧 婀垮害',unit:'%',dec:1},{key:'lux',label:'鈽€锔?鍏夌収',unit:'lx',dec:0},{key:'soil',label:'馃尡 鍦熷￥',unit:'%',dec:0},{key:'eco2',label:'鈽侊笍 eCO2',unit:'ppm',dec:0},{key:'tvoc',label:'馃И TVOC',unit:'ppb',dec:0}];
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
    }).catch(function(e){alert('鏌ヨ澶辫触: '+e.message);});
  };

  window.loadHistoryLog=function(){
    var date=document.getElementById('hist-date-select').value;if(!date)return;
    fetch('/api/log?date='+date).then(function(r){return r.json();}).then(function(data){
      if(data.error){alert(data.error);return;}
      document.getElementById('hist-charts-wrap').style.display='none';
      document.getElementById('hist-stats-wrap').style.display='none';
      document.getElementById('hist-log-wrap').style.display='';
      var list=document.getElementById('hist-log-list');
      if(!data.entries||data.entries.length===0){list.innerHTML='<div class="text-gray-400">褰撴棩鏃犺褰?/div>';}
      else{list.innerHTML=data.entries.map(function(e){return '<div class="py-1 border-b border-slate-200">'+e.replace(/</g,'&lt;')+'</div>';}).join('');}
    }).catch(function(e){alert('鏌ヨ澶辫触: '+e.message);});
  };

  // ===== QQ Bot =====
  function loadQQBotConfig(){
    fetch('/api/qqbot/config').then(function(r){return r.json();}).then(function(d){
      document.getElementById('qqbot-appid').value=d.appId||'';
      document.getElementById('qqbot-groupid').value=d.userOpenId||'';
      document.getElementById('qqbot-enabled').checked=d.enabled||false;
      var st=document.getElementById('qqbot-status');
      if(!d.appId){st.innerText='鏈厤缃?;st.style.color='#9ca3af';}
      else if(d.gatewayReady){st.innerText='Gateway 灏辩华';st.style.color='#16a34a';}
      else if(d.tokenValid){st.innerText='Token 鏈夋晥';st.style.color='#2563eb';}
      else if(d.enabled){st.innerText='宸插惎鐢?;st.style.color='#2563eb';}
      else{st.innerText='宸茬鐢?;st.style.color='#ea580c';}
      var dots=[document.getElementById('gw-status-dot'),document.getElementById('gw-dot-side')];
      var gwt=document.getElementById('gw-status-text');
      var gws=document.getElementById('gw-status-side');
      if(d.gatewayReady){
        dots.forEach(function(x){if(x)x.style.background='#22c55e';});
        if(gwt)gwt.innerText='宸茶繛鎺?;if(gws)gws.innerText='GW 鍦ㄧ嚎';
      }else if(d.gatewayConnected){
        dots.forEach(function(x){if(x)x.style.background='#f59e0b';});
        if(gwt)gwt.innerText='杩炴帴涓?..';if(gws)gws.innerText='GW 杩炴帴涓?;
      }else if(d.enabled){
        dots.forEach(function(x){if(x)x.style.background='#ef4444';});
        if(gwt)gwt.innerText='鏈繛鎺?;if(gws)gws.innerText='GW 绂荤嚎';
      }else{
        dots.forEach(function(x){if(x)x.style.background='#9ca3af';});
        if(gwt)gwt.innerText='鏈惎鐢?;if(gws)gws.innerText='GW 鏈惎鐢?;
      }
    }).catch(function(){});
  }
  setTimeout(loadQQBotConfig,3000);
  setInterval(loadQQBotConfig,15000);

  window.saveQQBotConfig=function(){
    var body={appId:document.getElementById('qqbot-appid').value,userOpenId:document.getElementById('qqbot-groupid').value,enabled:document.getElementById('qqbot-enabled').checked};
    var sec=document.getElementById('qqbot-secret').value;if(sec)body.appSecret=sec;
    fetch('/api/qqbot/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}).then(function(r){return r.json();}).then(function(d){
      if(d.ok){alert('閰嶇疆宸蹭繚瀛?);loadQQBotConfig();}
    }).catch(function(e){alert('淇濆瓨澶辫触: '+e.message);});
  };
  window.testQQBot=function(){
    fetch('/api/qqbot/test').then(function(r){return r.json();}).then(function(d){
      alert(d.ok?'娴嬭瘯娑堟伅鍙戦€佹垚鍔燂紒':('澶辫触: '+(d.error||'鏈煡閿欒')));
    }).catch(function(e){alert('璇锋眰澶辫触: '+e.message);});
  };

  // ===== Resize =====
  window.addEventListener('resize',function(){chartTH.resize();chartLS.resize();chartAQ.resize();});
})();
</script>
</body>
</html>
)rawliteral";
