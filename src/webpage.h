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
.main-wrap{margin-left:var(--sidebar-w);min-height:100vh;padding:24px;transition:margin .3s}
.topbar{display:none;position:fixed;top:0;left:0;right:0;height:56px;background:#fff;border-bottom:1px solid #e2e8f0;z-index:40;padding:0 16px;align-items:center;justify-content:space-between}
.topbar .menu-btn{width:36px;height:36px;border:none;background:#f1f5f9;border-radius:8px;font-size:18px;cursor:pointer}
.bottom-tabs{display:none;position:fixed;bottom:0;left:0;right:0;height:60px;background:#fff;border-top:1px solid #e2e8f0;z-index:40;justify-content:space-around;align-items:center;padding-bottom:env(safe-area-inset-bottom)}
.tab-btn{display:flex;flex-direction:column;align-items:center;gap:2px;border:none;background:none;color:#94a3b8;font-size:10px;cursor:pointer;padding:4px 8px}
.tab-btn.active{color:var(--primary)}
.tab-btn .t-icon{font-size:20px}
.panel{display:none}
.panel.active{display:block}
.card{background:#fff;border-radius:16px;box-shadow:0 1px 3px rgba(0,0,0,0.06);padding:20px;border:1px solid #f1f5f9}
.card-sm{padding:16px}
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
.dev-card{display:flex;align-items:center;justify-content:space-between;padding:16px 20px}
.dev-info{display:flex;align-items:center;gap:12px}
.dev-icon{width:42px;height:42px;border-radius:12px;display:flex;align-items:center;justify-content:center;font-size:22px}
.toggle{position:relative;width:44px;height:24px;border-radius:12px;background:#cbd5e1;cursor:pointer;transition:.2s}
.toggle.on{background:#3b82f6}
.toggle::after{content:'';position:absolute;top:2px;left:2px;width:20px;height:20px;border-radius:50%;background:#fff;transition:.2s;box-shadow:0 1px 3px rgba(0,0,0,.2)}
.toggle.on::after{left:22px}
.toggle.disabled{opacity:.5;cursor:not-allowed}
.chart-box{height:280px;border-radius:12px;background:#f8fafc;padding:4px}
.alert-item{padding:8px 12px;border-radius:8px;font-size:13px;margin-bottom:6px}
.alert-danger{background:#fef2f2;color:#dc2626;border-left:3px solid #ef4444}
.alert-warn{background:#fffbeb;color:#d97706;border-left:3px solid #f59e0b}
.ai-float-btn{position:fixed;bottom:80px;right:20px;width:52px;height:52px;border-radius:50%;background:linear-gradient(135deg,#3b82f6,#8b5cf6);color:#fff;border:none;font-size:24px;cursor:pointer;box-shadow:0 4px 12px rgba(59,130,246,.4);z-index:35;display:flex;align-items:center;justify-content:center;transition:transform .2s}
.ai-float-btn:hover{transform:scale(1.1)}
.ai-drawer{position:fixed;bottom:0;right:0;width:380px;max-width:100vw;height:70vh;max-height:600px;background:#fff;border-radius:16px 16px 0 0;box-shadow:-4px -4px 20px rgba(0,0,0,.12);z-index:55;display:flex;flex-direction:column;transform:translateY(100%);transition:transform .3s;overflow:hidden}
.ai-drawer.open{transform:translateY(0)}
.ai-drawer-head{padding:14px 16px;background:linear-gradient(135deg,#3b82f6,#8b5cf6);color:#fff;display:flex;align-items:center;justify-content:space-between;flex-shrink:0}
.ai-drawer-body{flex:1;overflow-y:auto;padding:12px}
.ai-drawer-foot{padding:10px;border-top:1px solid #e2e8f0;display:flex;gap:8px}
@media(max-width:768px){
  .sidebar{transform:translateX(-100%)}
  .sidebar.open{transform:translateX(0)}
  .sidebar-overlay{display:none;position:fixed;inset:0;background:rgba(0,0,0,.4);z-index:45}
  .sidebar-overlay.show{display:block}
  .main-wrap{margin-left:0;padding:72px 12px 76px}
  .topbar{display:flex}
  .bottom-tabs{display:flex}
  .ai-float-btn{bottom:72px;right:12px;width:46px;height:46px;font-size:20px}
  .ai-drawer{width:100%;height:60vh;border-radius:16px 16px 0 0}
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
<div class="sidebar-overlay" id="sidebar-overlay" onclick="closeSidebar()"></div>
<nav class="sidebar" id="sidebar">
  <div class="sidebar-brand">
    <div class="logo">&#x1F331;</div>
    <div>
      <div class="brand-text">AI-Biodome</div>
      <div class="brand-sub">&#x667A;&#x6167;&#x5927;&#x68DA;&#x63A7;&#x5236;&#x53F0;</div>
    </div>
  </div>
  <div class="nav-list">
    <button class="nav-item active" onclick="switchTab('dashboard')" data-tab="dashboard">
      <span class="icon">&#x1F4CA;</span><span class="nav-label">&#x4EEA;&#x8868;&#x76D8;</span>
    </button>
    <button class="nav-item" onclick="switchTab('control')" data-tab="control">
      <span class="icon">&#x2699;&#xFE0F;</span><span class="nav-label">&#x8BBE;&#x5907;&#x63A7;&#x5236;</span>
    </button>
    <button class="nav-item" onclick="switchTab('history')" data-tab="history">
      <span class="icon">&#x1F4C1;</span><span class="nav-label">&#x5386;&#x53F2;&#x6570;&#x636E;</span>
    </button>
    <button class="nav-item" onclick="switchTab('qqbot')" data-tab="qqbot">
      <span class="icon">&#x1F916;</span><span class="nav-label">QQ &#x673A;&#x5668;&#x4EBA;</span>
    </button>
    <button class="nav-item" onclick="switchTab('system')" data-tab="system">
      <span class="icon">&#x1F4CB;</span><span class="nav-label">&#x7CFB;&#x7EDF;&#x4FE1;&#x606F;</span>
    </button>
  </div>
  <div class="sidebar-footer">
    <div class="conn-badge">
      <span class="conn-dot off" id="ws-dot"></span>
      <span id="ws-status-text">&#x672A;&#x8FDE;&#x63A5;</span>
    </div>
    <div class="conn-badge" style="margin-top:4px">
      <span class="conn-dot off" id="gw-dot-side"></span>
      <span id="gw-status-side">Gateway</span>
    </div>
  </div>
</nav>
<div class="topbar" id="topbar">
  <button class="menu-btn" onclick="openSidebar()">&#x2630;</button>
  <span class="font-bold text-sm text-gray-700">&#x1F331; AI-Biodome</span>
  <div class="flex items-center gap-2">
    <span class="conn-dot off" id="ws-dot-mobile" style="width:8px;height:8px;border-radius:50%"></span>
  </div>
</div>
<div class="bottom-tabs" id="bottom-tabs">
  <button class="tab-btn active" onclick="switchTab('dashboard')" data-tab="dashboard"><span class="t-icon">&#x1F4CA;</span>&#x4EEA;&#x8868;&#x76D8;</button>
  <button class="tab-btn" onclick="switchTab('control')" data-tab="control"><span class="t-icon">&#x2699;&#xFE0F;</span>&#x63A7;&#x5236;</button>
  <button class="tab-btn" onclick="switchTab('history')" data-tab="history"><span class="t-icon">&#x1F4C1;</span>&#x5386;&#x53F2;</button>
  <button class="tab-btn" onclick="switchTab('qqbot')" data-tab="qqbot"><span class="t-icon">&#x1F916;</span>&#x673A;&#x5668;&#x4EBA;</button>
  <button class="tab-btn" onclick="switchTab('system')" data-tab="system"><span class="t-icon">&#x1F4CB;</span>&#x7CFB;&#x7EDF;</button>
</div>
<button class="ai-float-btn" id="ai-float-btn" onclick="toggleAIChat()">&#x1F4AC;</button>
<div class="ai-drawer" id="ai-drawer">
  <div class="ai-drawer-head">
    <div class="flex items-center gap-2"><span class="text-lg">&#x1F9E0;</span><span class="font-bold text-sm">AI &#x667A;&#x80FD;&#x52A9;&#x624B;</span></div>
    <div class="flex items-center gap-2">
      <button id="btn-ai-analyze" onclick="askAI()" class="px-3 py-1 bg-white/20 hover:bg-white/30 rounded-lg text-xs font-medium transition-colors">&#x73AF;&#x5883;&#x5206;&#x6790;</button>
      <button onclick="toggleAIChat()" class="text-white/80 hover:text-white text-lg" style="background:none;border:none;cursor:pointer">&#x2715;</button>
    </div>
  </div>
  <div class="ai-drawer-body" id="ai-chat-box">
    <div class="flex justify-start pl-2 pr-8">
      <div class="bg-gray-50 border border-gray-100 p-3 rounded-2xl rounded-tl-none text-sm text-gray-600">&#x4F60;&#x597D;&#xFF01;&#x6211;&#x662F; AI &#x52A9;&#x624B;&#xFF0C;&#x53EF;&#x4EE5;&#x5E2E;&#x4F60;&#x5206;&#x6790;&#x5927;&#x68DA;&#x73AF;&#x5883;&#x6570;&#x636E;&#x3002;&#x70B9;&#x51FB;&#x201C;&#x73AF;&#x5883;&#x5206;&#x6790;&#x201D;&#x6216;&#x76F4;&#x63A5;&#x63D0;&#x95EE;&#x3002;</div>
    </div>
  </div>
  <div class="ai-drawer-foot">
    <input type="text" id="ai-input" placeholder="&#x8F93;&#x5165;&#x4F60;&#x7684;&#x95EE;&#x9898;..." class="flex-1 px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
    <button onclick="sendAIChat()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">&#x53D1;&#x9001;</button>
  </div>
</div>
<div class="main-wrap" id="main-wrap">
<div class="panel active" id="panel-dashboard">
  <div class="flex items-center justify-between mb-5">
    <div>
      <h1 class="text-xl font-bold text-gray-800">&#x5B9E;&#x65F6;&#x76D1;&#x63A7;</h1>
      <p class="text-xs text-gray-400 mt-1" id="val-time-sync">&#x7CFB;&#x7EDF;&#x65F6;&#x95F4;: --</p>
    </div>
    <div class="flex items-center gap-3">
      <span class="text-xs px-3 py-1 rounded-full bg-blue-50 text-blue-600 font-medium" id="mode-badge">&#x81EA;&#x52A8;&#x6A21;&#x5F0F;</span>
    </div>
  </div>
  <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4 mb-6">
    <div class="card card-sm sensor-card sc-temp">
      <div class="flex items-center justify-between"><span class="sensor-icon">&#x1F321;&#xFE0F;</span><span class="s-badge" id="badge-temp">--</span></div>
      <div class="sensor-val" id="val-temp">-- &#xB0;C</div>
      <div class="sensor-label">&#x6E29;&#x5EA6;</div>
    </div>
    <div class="card card-sm sensor-card sc-hum">
      <div class="flex items-center justify-between"><span class="sensor-icon">&#x1F4A7;</span><span class="s-badge" id="badge-hum">--</span></div>
      <div class="sensor-val" id="val-hum">-- %</div>
      <div class="sensor-label">&#x6E7F;&#x5EA6;</div>
    </div>
    <div class="card card-sm sensor-card sc-lux">
      <div class="flex items-center justify-between"><span class="sensor-icon">&#x2600;&#xFE0F;</span><span class="s-badge" id="badge-lux">--</span></div>
      <div class="sensor-val" id="val-lux">-- lx</div>
      <div class="sensor-label">&#x5149;&#x7167;</div>
    </div>
    <div class="card card-sm sensor-card sc-soil">
      <div class="flex items-center justify-between"><span class="sensor-icon">&#x1F331;</span><span class="s-badge" id="badge-soil">--</span></div>
      <div class="sensor-val" id="val-soil">-- %</div>
      <div class="sensor-label">&#x571F;&#x58E4;&#x6C34;&#x5206;</div>
    </div>
    <div class="card card-sm sensor-card sc-co2">
      <div class="flex items-center justify-between"><span class="sensor-icon">&#x2601;&#xFE0F;</span><span class="s-badge" id="badge-eco2">--</span></div>
      <div class="sensor-val" id="val-eco2">-- ppm</div>
      <div class="sensor-label">eCO2</div>
    </div>
    <div class="card card-sm sensor-card sc-tvoc">
      <div class="flex items-center justify-between"><span class="sensor-icon">&#x1F9EA;</span><span class="s-badge" id="badge-tvoc">--</span></div>
      <div class="sensor-val" id="val-tvoc">-- ppb</div>
      <div class="sensor-label">TVOC</div>
    </div>
  </div>
  <div class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">&#x26A1; &#x73AF;&#x5883;&#x9884;&#x8B66;</h3>
    <div id="alert-list"><div class="text-green-600 text-sm font-medium">&#x2705; &#x73AF;&#x5883;&#x72B6;&#x6001;&#x826F;&#x597D;</div></div>
  </div>
  <div class="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
    <div class="card card-sm"><div class="chart-box" id="chart-th"></div></div>
    <div class="card card-sm"><div class="chart-box" id="chart-ls"></div></div>
    <div class="card card-sm"><div class="chart-box" id="chart-aq"></div></div>
  </div>
  <div class="card">
    <h3 class="text-sm font-bold text-gray-600 mb-3">&#x1F4C8; &#x5B9E;&#x65F6;&#x7EDF;&#x8BA1;</h3>
    <div class="overflow-x-auto">
      <table class="w-full text-sm text-center">
        <thead><tr class="bg-slate-50 text-gray-500 text-xs">
          <th class="px-3 py-2 text-left font-semibold">&#x6307;&#x6807;</th>
          <th class="px-3 py-2 font-semibold">&#x6700;&#x5C0F;</th>
          <th class="px-3 py-2 font-semibold">&#x6700;&#x5927;</th>
          <th class="px-3 py-2 font-semibold">&#x5E73;&#x5747;</th>
          <th class="px-3 py-2 font-semibold">&#x5F53;&#x524D;</th>
        </tr></thead>
        <tbody id="stats-body"></tbody>
      </table>
    </div>
  </div>
</div>
<div class="panel" id="panel-control">
  <h1 class="text-xl font-bold text-gray-800 mb-5">&#x8BBE;&#x5907;&#x63A7;&#x5236;</h1>
  <div class="card mb-6">
    <div class="flex items-center justify-between">
      <div>
        <h3 class="font-bold text-gray-800">&#x8FD0;&#x884C;&#x6A21;&#x5F0F;</h3>
        <p class="text-xs text-gray-400 mt-1">&#x81EA;&#x52A8;&#x6A21;&#x5F0F;&#x4E0B;&#x8BBE;&#x5907;&#x6839;&#x636E;&#x4F20;&#x611F;&#x5668;&#x6570;&#x636E;&#x81EA;&#x52A8;&#x8C03;&#x8282;</p>
      </div>
      <div class="flex items-center gap-3">
        <span class="text-sm font-bold" id="mode-text" style="color:#16a34a">&#x81EA;&#x52A8;</span>
        <label class="relative inline-flex items-center cursor-pointer">
          <input type="checkbox" id="cb-mode" class="sr-only peer" checked onchange="toggleMode()">
          <div class="w-12 h-6 bg-gray-300 peer-focus:outline-none rounded-full peer peer-checked:bg-green-500 peer-checked:after:translate-x-6 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all after:shadow"></div>
        </label>
      </div>
    </div>
  </div>
  <div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-6">
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-blue-50"><span>&#x1F4A7;</span></div>
        <div><div class="font-bold text-gray-800 text-sm">&#x6C34;&#x6CF5;</div><div class="text-xs text-gray-400">&#x571F;&#x58E4;&#x704C;&#x6E89;</div></div>
      </div>
      <div class="toggle disabled" id="cb-pump" onclick="toggleDevice('pump')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-yellow-50"><span>&#x1F4A1;</span></div>
        <div><div class="font-bold text-gray-800 text-sm">&#x8865;&#x5149;&#x706F;</div><div class="text-xs text-gray-400">&#x690D;&#x7269;&#x8865;&#x5149;</div></div>
      </div>
      <div class="toggle disabled" id="cb-light" onclick="toggleDevice('light')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-red-50"><span>&#x1F525;</span></div>
        <div><div class="font-bold text-gray-800 text-sm">&#x52A0;&#x70ED;&#x57AB;</div><div class="text-xs text-gray-400">&#x6E29;&#x5EA6;&#x8C03;&#x8282;</div></div>
      </div>
      <div class="toggle disabled" id="cb-heater" onclick="toggleDevice('heater')"></div>
    </div>
    <div class="card dev-card">
      <div class="dev-info">
        <div class="dev-icon bg-cyan-50"><span>&#x1F300;</span></div>
        <div><div class="font-bold text-gray-800 text-sm">&#x6392;&#x98CE;&#x6247;</div><div class="text-xs text-gray-400">&#x901A;&#x98CE;&#x6362;&#x6C14;</div></div>
      </div>
      <div class="toggle disabled" id="cb-fan" onclick="toggleDevice('fan')"></div>
    </div>
  </div>
  <div class="card">
    <h3 class="font-bold text-gray-800 mb-4">&#x23F0; &#x5B9A;&#x65F6;&#x4EFB;&#x52A1;</h3>
    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
      <div class="bg-slate-50 rounded-xl p-4">
        <div class="flex items-center justify-between mb-3">
          <span class="font-semibold text-sm text-gray-700">&#x1F300; &#x5B9A;&#x65F6;&#x901A;&#x98CE;</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="sched-fan-en" class="sr-only peer" onchange="saveSchedule()">
            <div class="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
        <div class="flex items-center gap-2 text-sm">
          <input type="time" id="sched-fan-start" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
          <span class="text-gray-400">&#x81F3;</span>
          <input type="time" id="sched-fan-end" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
        </div>
      </div>
      <div class="bg-slate-50 rounded-xl p-4">
        <div class="flex items-center justify-between mb-3">
          <span class="font-semibold text-sm text-gray-700">&#x1F4A1; &#x5B9A;&#x65F6;&#x8865;&#x5149;</span>
          <label class="relative inline-flex items-center cursor-pointer">
            <input type="checkbox" id="sched-light-en" class="sr-only peer" onchange="saveSchedule()">
            <div class="w-10 h-5 bg-gray-300 rounded-full peer peer-checked:bg-blue-500 peer-checked:after:translate-x-5 after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-4 after:w-4 after:transition-all"></div>
          </label>
        </div>
        <div class="flex items-center gap-2 text-sm">
          <input type="time" id="sched-light-start" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
          <span class="text-gray-400">&#x81F3;</span>
          <input type="time" id="sched-light-end" class="px-2 py-1 rounded border text-sm" value="00:00" onchange="saveSchedule()">
        </div>
      </div>
    </div>
  </div>
</div>
<div class="panel" id="panel-history">
  <h1 class="text-xl font-bold text-gray-800 mb-5">&#x5386;&#x53F2;&#x6570;&#x636E;</h1>
  <div class="card mb-6">
    <div class="flex flex-wrap items-center gap-3 mb-3">
      <select id="hist-date-select" class="px-3 py-2 rounded-lg border text-sm focus:ring-2 focus:ring-blue-400 outline-none">
        <option value="">&#x52A0;&#x8F7D;&#x4E2D;...</option>
      </select>
      <button onclick="loadHistoryData()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">&#x67E5;&#x8BE2;&#x6570;&#x636E;</button>
      <button onclick="loadHistoryLog()" class="px-4 py-2 bg-gray-500 hover:bg-gray-600 text-white rounded-lg text-sm font-medium transition-colors">&#x67E5;&#x770B;&#x65E5;&#x5FD7;</button>
    </div>
    <div class="text-xs text-gray-400" id="storage-info">&#x5B58;&#x50A8;&#x72B6;&#x6001;&#x52A0;&#x8F7D;&#x4E2D;...</div>
  </div>
  <div class="grid grid-cols-1 md:grid-cols-2 gap-4 mb-6">
    <div class="card card-sm">
      <h3 class="text-sm font-bold text-gray-600 mb-3">&#x1F4CA; &#x8BB0;&#x5F55;&#x7C92;&#x5EA6;</h3>
      <div class="flex items-center gap-3">
        <input type="number" id="save-interval" min="10" max="3600" step="10" value="300" class="w-24 px-2 py-1 rounded border text-sm text-center focus:ring-2 focus:ring-blue-400 outline-none">
        <span class="text-sm text-gray-500">&#x79D2;</span>
        <button onclick="saveSaveInterval()" class="px-3 py-1 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-xs font-medium transition-colors">&#x4FDD;&#x5B58;</button>
        <span class="text-xs text-gray-400" id="interval-hint">&#x9ED8;&#x8BA4; 300&#x79D2;(5&#x5206;&#x949F;)</span>
      </div>
    </div>
    <div class="card card-sm">
      <h3 class="text-sm font-bold text-gray-600 mb-3">&#x1F4E5; &#x6570;&#x636E;&#x5BFC;&#x51FA;</h3>
      <div class="flex flex-wrap items-center gap-2">
        <select id="export-start" class="px-2 py-1 rounded border text-sm"></select>
        <span class="text-gray-400 text-sm">&#x81F3;</span>
        <select id="export-end" class="px-2 py-1 rounded border text-sm"></select>
        <button onclick="exportCSV()" class="px-3 py-1 bg-green-500 hover:bg-green-600 text-white rounded-lg text-xs font-medium transition-colors">&#x5BFC;&#x51FA; CSV</button>
      </div>
    </div>
  </div>
  <div id="hist-charts-wrap" style="display:none" class="grid grid-cols-1 lg:grid-cols-3 gap-4 mb-6">
    <div class="card card-sm"><div class="chart-box" id="hist-chart-th"></div></div>
    <div class="card card-sm"><div class="chart-box" id="hist-chart-ls"></div></div>
    <div class="card card-sm"><div class="chart-box" id="hist-chart-aq"></div></div>
  </div>
  <div id="hist-stats-wrap" style="display:none" class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">&#x5F53;&#x65E5;&#x7EDF;&#x8BA1;&#x6458;&#x8981;</h3>
    <div class="overflow-x-auto">
      <table class="w-full text-sm text-center">
        <thead><tr class="bg-slate-50 text-gray-500 text-xs">
          <th class="px-3 py-2 text-left font-semibold">&#x6307;&#x6807;</th>
          <th class="px-3 py-2 font-semibold">&#x6700;&#x5C0F;</th>
          <th class="px-3 py-2 font-semibold">&#x6700;&#x5927;</th>
          <th class="px-3 py-2 font-semibold">&#x5E73;&#x5747;</th>
          <th class="px-3 py-2 font-semibold">&#x6570;&#x636E;&#x70B9;</th>
        </tr></thead>
        <tbody id="hist-stats-body"></tbody>
      </table>
    </div>
  </div>
  <div id="hist-log-wrap" style="display:none" class="card">
    <h3 class="text-sm font-bold text-gray-600 mb-3">&#x64CD;&#x4F5C;&#x65E5;&#x5FD7;</h3>
    <div id="hist-log-list" class="bg-slate-50 rounded-lg p-4 max-h-80 overflow-y-auto text-xs font-mono text-gray-700 space-y-1"></div>
  </div>
</div>
<div class="panel" id="panel-qqbot">
  <h1 class="text-xl font-bold text-gray-800 mb-5">QQ &#x673A;&#x5668;&#x4EBA;</h1>
  <div class="card mb-6">
    <div class="flex items-center justify-between mb-4">
      <div class="flex items-center gap-3">
        <span class="text-2xl">&#x1F916;</span>
        <div>
          <h3 class="font-bold text-gray-800">&#x673A;&#x5668;&#x4EBA;&#x914D;&#x7F6E;</h3>
          <span class="text-xs font-medium" id="qqbot-status">&#x672A;&#x914D;&#x7F6E;</span>
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
        <label class="block text-xs font-semibold text-gray-500 mb-1">&#x7528;&#x6237; OpenID&#xFF08;&#x53EF;&#x9009;&#xFF09;</label>
        <input type="text" id="qqbot-groupid" placeholder="&#x4E3B;&#x52A8;&#x63A8;&#x9001;&#x7528;" class="w-full px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none">
      </div>
    </div>
    <div class="flex flex-wrap items-center gap-3">
      <button onclick="saveQQBotConfig()" class="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">&#x4FDD;&#x5B58;&#x914D;&#x7F6E;</button>
      <button onclick="testQQBot()" class="px-4 py-2 bg-green-500 hover:bg-green-600 text-white rounded-lg text-sm font-medium transition-colors">&#x53D1;&#x9001;&#x6D4B;&#x8BD5;</button>
    </div>
  </div>
  <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
    <div class="card">
      <h3 class="text-sm font-bold text-gray-600 mb-3">Gateway &#x72B6;&#x6001;</h3>
      <div class="flex items-center gap-3 mb-3">
        <span class="w-3 h-3 rounded-full" id="gw-status-dot" style="background:#9ca3af"></span>
        <span class="text-sm font-medium" id="gw-status-text">&#x672A;&#x8FDE;&#x63A5;</span>
      </div>
      <p class="text-xs text-gray-400">Gateway &#x8FDE;&#x63A5;&#x540E;&#xFF0C;&#x7528;&#x6237;&#x53EF;&#x79C1;&#x804A;&#x673A;&#x5668;&#x4EBA;&#x67E5;&#x8BE2;&#x5927;&#x68DA;&#x72B6;&#x6001;</p>
    </div>
    <div class="card">
      <h3 class="text-sm font-bold text-gray-600 mb-3">&#x652F;&#x6301;&#x7684;&#x6307;&#x4EE4;</h3>
      <div class="space-y-2">
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">&#x72B6;&#x6001;</code><span class="text-gray-600">&#x67E5;&#x770B;&#x73AF;&#x5883;&#x6570;&#x636E;</span></div>
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">&#x544A;&#x8B66;</code><span class="text-gray-600">&#x67E5;&#x770B;&#x5F53;&#x524D;&#x544A;&#x8B66;</span></div>
        <div class="flex items-center gap-2 text-sm"><code class="bg-slate-100 px-2 py-0.5 rounded text-blue-600 font-mono text-xs">&#x5E2E;&#x52A9;</code><span class="text-gray-600">&#x663E;&#x793A;&#x6307;&#x4EE4;&#x5217;&#x8868;</span></div>
      </div>
    </div>
  </div>
</div>
<div class="panel" id="panel-system">
  <h1 class="text-xl font-bold text-gray-800 mb-5">&#x7CFB;&#x7EDF;&#x4FE1;&#x606F;</h1>
  <div class="card mb-6">
    <h3 class="text-sm font-bold text-gray-600 mb-3">&#x4F20;&#x611F;&#x5668;&#x72B6;&#x6001;</h3>
    <div class="grid grid-cols-2 md:grid-cols-4 gap-3">
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-sht40"></span>
        <span class="text-sm text-gray-600">SHT40 &#x6E29;&#x6E7F;&#x5EA6;</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-bh1750"></span>
        <span class="text-sm text-gray-600">BH1750 &#x5149;&#x7167;</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot off" id="dot-sgp30"></span>
        <span class="text-sm text-gray-600">SGP30 &#x7A7A;&#x6C14;</span>
      </div>
      <div class="flex items-center gap-2 bg-slate-50 rounded-lg p-3">
        <span class="conn-dot on" id="dot-soil"></span>
        <span class="text-sm text-gray-600">ADC &#x571F;&#x58E4;</span>
      </div>
    </div>
  </div>
  <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">&#x7CFB;&#x7EDF;&#x65F6;&#x95F4;</div>
      <div class="text-lg font-bold text-gray-800" id="val-sys-time">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">&#x5185;&#x5B58;&#x4F7F;&#x7528;</div>
      <div class="text-lg font-bold text-gray-800" id="val-heap">-- %</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">WiFi &#x4FE1;&#x53F7;</div>
      <div class="text-lg font-bold text-gray-800"><span id="val-rssi">--</span> <span class="text-sm font-normal" id="val-rssi-quality"></span></div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">&#x8FD0;&#x884C;&#x65F6;&#x95F4;</div>
      <div class="text-lg font-bold text-gray-800" id="val-uptime">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">&#x82AF;&#x7247;&#x4FE1;&#x606F;</div>
      <div class="text-sm font-bold text-gray-800"><span id="val-chip-rev">--</span> &#xB7; <span id="val-cpu-cores">--</span> &#xB7; <span id="val-cpu-freq">--</span></div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">&#x7F51;&#x7EDC;</div>
      <div class="text-sm font-bold text-gray-800" id="val-ip">--</div>
      <div class="text-xs text-gray-400 mt-1" id="val-mac">--</div>
    </div>
    <div class="card card-sm">
      <div class="text-xs text-gray-400 mb-1">SDK &#x7248;&#x672C;</div>
      <div class="text-sm font-bold text-gray-800" id="val-sdk">--</div>
    </div>
  </div>
</div>
</div>
<script>
(function(){
  var currentTab='dashboard';
  window.switchTab=function(tab){
    currentTab=tab;
    document.querySelectorAll('.panel').forEach(function(p){p.classList.remove('active');});
    document.getElementById('panel-'+tab).classList.add('active');
    document.querySelectorAll('.nav-item').forEach(function(n){n.classList.toggle('active',n.getAttribute('data-tab')===tab);});
    document.querySelectorAll('.tab-btn').forEach(function(n){n.classList.toggle('active',n.getAttribute('data-tab')===tab);});
    closeSidebar();
    if(tab==='dashboard'){setTimeout(function(){chartTH.resize();chartLS.resize();chartAQ.resize();},50);}
    if(tab==='history'&&histChartTH){setTimeout(function(){histChartTH.resize();histChartLS.resize();histChartAQ.resize();},50);}
  };
  window.openSidebar=function(){document.getElementById('sidebar').classList.add('open');document.getElementById('sidebar-overlay').classList.add('show');};
  window.closeSidebar=function(){document.getElementById('sidebar').classList.remove('open');document.getElementById('sidebar-overlay').classList.remove('show');};
  var aiOpen=false;
  window.toggleAIChat=function(){aiOpen=!aiOpen;document.getElementById('ai-drawer').classList.toggle('open',aiOpen);};

  var chartTH=echarts.init(document.getElementById('chart-th'));
  var chartLS=echarts.init(document.getElementById('chart-ls'));
  var chartAQ=echarts.init(document.getElementById('chart-aq'));
  function makeChartOpt(title,names,colors,yNames,yMin){
    return{title:{text:title,left:'center',textStyle:{color:'#475569',fontSize:13}},tooltip:{trigger:'axis'},legend:{data:names,top:22,itemWidth:10,itemHeight:10,textStyle:{fontSize:11}},grid:{left:'14%',right:'14%',bottom:'10%',top:'28%'},xAxis:{type:'category',data:[]},yAxis:[{type:'value',name:yNames[0],min:yMin||undefined},{type:'value',name:yNames[1],position:'right',min:yNames[1]==='%'?0:undefined,max:yNames[1]==='%'?100:undefined}],series:[{name:names[0],type:'line',data:[],smooth:true,itemStyle:{color:colors[0]},areaStyle:{opacity:0.08},symbol:'none'},{name:names[1],type:'line',yAxisIndex:1,data:[],smooth:true,itemStyle:{color:colors[1]},areaStyle:{opacity:0.08},symbol:'none'}]};
  }
  chartTH.setOption(makeChartOpt('\u6E29\u6E7F\u5EA6',['\u6E29\u5EA6(\u00B0C)','\u6E7F\u5EA6(%)'],['#ef4444','#3b82f6'],['\u00B0C','%']));
  chartLS.setOption(makeChartOpt('\u5149\u7167\u00B7\u571F\u58E4',['\u5149\u7167(lx)','\u571F\u58E4(%)'],['#f97316','#22c55e'],['lx','%']));
  chartAQ.setOption(makeChartOpt('\u7A7A\u6C14\u8D28\u91CF',['eCO2(ppm)','TVOC(ppb)'],['#14b8a6','#a855f7'],['ppm','ppb'],400));

  var gateway='ws://'+window.location.hostname+':81/';
  var socket=null;var reconnectTimer=null;
  function setWsStatus(s){
    var dots=[document.getElementById('ws-dot'),document.getElementById('ws-dot-mobile')];
    var text=document.getElementById('ws-status-text');
    dots.forEach(function(d){if(!d)return;d.className='conn-dot '+(s==='on'?'on':s==='pending'?'pending':'off');});
    if(text)text.innerText=s==='on'?'\u5DF2\u8FDE\u63A5':s==='pending'?'\u8FDE\u63A5\u4E2D':'\u672A\u8FDE\u63A5';
  }
  function connectWS(){
    if(reconnectTimer)clearTimeout(reconnectTimer);
    if(socket&&socket.readyState!==WebSocket.CLOSED)socket.close();
    setWsStatus('pending');
    socket=new WebSocket(gateway);
    socket.onopen=function(){setWsStatus('on');};
    socket.onmessage=function(e){try{updateUI(JSON.parse(e.data));}catch(err){}};
    socket.onerror=function(){setWsStatus('off');};
    socket.onclose=function(){setWsStatus('off');reconnectTimer=setTimeout(connectWS,3000);};
  }
  connectWS();
  document.addEventListener('visibilitychange',function(){if(!document.hidden&&(!socket||socket.readyState===WebSocket.CLOSED))connectWS();});

  function setBadge(id,val,low,warnH,high){
    var el=document.getElementById(id);if(!el)return;
    if(val==null){el.className='s-badge';el.innerText='--';return;}
    if(val<low){el.className='s-badge b-low';el.innerText='\u504F\u4F4E';}
    else if(val>high){el.className='s-badge b-high';el.innerText='\u504F\u9AD8';}
    else if(val>warnH){el.className='s-badge b-warn';el.innerText='\u6CE8\u610F';}
    else{el.className='s-badge b-ok';el.innerText='\u6B63\u5E38';}
  }
  function fmtUptime(s){
    var d=Math.floor(s/86400),h=Math.floor((s%86400)/3600),m=Math.floor((s%3600)/60),sec=s%60;
    if(d>0)return d+'\u5929 '+h+'\u65F6 '+m+'\u5206';
    if(h>0)return h+'\u65F6 '+m+'\u5206 '+sec+'\u79D2';
    return m+'\u5206 '+sec+'\u79D2';
  }
  function calcStats(arr){
    if(!arr||arr.length===0)return null;
    var min=Infinity,max=-Infinity,sum=0;
    for(var i=0;i<arr.length;i++){var v=parseFloat(arr[i]);if(v<min)min=v;if(v>max)max=v;sum+=v;}
    return{min:min,max:max,avg:sum/arr.length};
  }
  function buildStatsRows(history,current,tbodyId){
    var metrics=[
      {key:'temp',label:'\u{1F321}\uFE0F \u6E29\u5EA6',unit:'\u00B0C',dec:1},
      {key:'hum',label:'\u{1F4A7} \u6E7F\u5EA6',unit:'%',dec:1},
      {key:'lux',label:'\u2600\uFE0F \u5149\u7167',unit:'lx',dec:0},
      {key:'soil',label:'\u{1F331} \u571F\u58E4',unit:'%',dec:0},
      {key:'eco2',label:'\u2601\uFE0F eCO2',unit:'ppm',dec:0},
      {key:'tvoc',label:'\u{1F9EA} TVOC',unit:'ppb',dec:0}
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
    if(el&&rows)el.innerHTML=rows;
  }
  function updateAlerts(c){
    var a=[];
    if(c.temp!=null){
      if(c.temp>35)a.push({cls:'alert-danger',msg:'\u26A0\uFE0F \u6E29\u5EA6\u8FC7\u9AD8 ('+c.temp.toFixed(1)+'\u00B0C)'});
      else if(c.temp<10)a.push({cls:'alert-danger',msg:'\u{1F976} \u6E29\u5EA6\u8FC7\u4F4E ('+c.temp.toFixed(1)+'\u00B0C)'});
      else if(c.temp>28)a.push({cls:'alert-warn',msg:'\u{1F321}\uFE0F \u6E29\u5EA6\u504F\u9AD8 ('+c.temp.toFixed(1)+'\u00B0C)'});
      else if(c.temp<15)a.push({cls:'alert-warn',msg:'\u{1F321}\uFE0F \u6E29\u5EA6\u504F\u4F4E ('+c.temp.toFixed(1)+'\u00B0C)'});
    }
    if(c.hum!=null){
      if(c.hum>90)a.push({cls:'alert-danger',msg:'\u{1F4A7} \u6E7F\u5EA6\u8FC7\u9AD8 ('+c.hum.toFixed(1)+'%)'});
      else if(c.hum<20)a.push({cls:'alert-warn',msg:'\u{1F4A7} \u6E7F\u5EA6\u504F\u4F4E ('+c.hum.toFixed(1)+'%)'});
    }
    if(c.soil!=null){
      if(c.soil<20)a.push({cls:'alert-danger',msg:'\u{1F331} \u571F\u58E4\u4E25\u91CD\u7F3A\u6C34 ('+c.soil+'%)'});
      else if(c.soil<30)a.push({cls:'alert-warn',msg:'\u{1F331} \u571F\u58E4\u504F\u5E72 ('+c.soil+'%)'});
    }
    if(c.eco2!=null){
      if(c.eco2>1500)a.push({cls:'alert-danger',msg:'\u2601\uFE0F CO2\u8D85\u6807 ('+c.eco2+'ppm)'});
      else if(c.eco2>1000)a.push({cls:'alert-warn',msg:'\u2601\uFE0F CO2\u504F\u9AD8 ('+c.eco2+'ppm)'});
    }
    if(c.tvoc!=null){
      if(c.tvoc>660)a.push({cls:'alert-danger',msg:'\u{1F9EA} TVOC\u8D85\u6807 ('+c.tvoc+'ppb)'});
      else if(c.tvoc>220)a.push({cls:'alert-warn',msg:'\u{1F9EA} TVOC\u504F\u9AD8 ('+c.tvoc+'ppb)'});
    }
    var el=document.getElementById('alert-list');
    el.innerHTML=a.length===0?'<div class="text-green-600 text-sm font-medium">\u2705 \u73AF\u5883\u72B6\u6001\u826F\u597D</div>':
      a.map(function(x){return'<div class="alert-item '+x.cls+'">'+x.msg+'</div>';}).join('');
  }

  function updateUI(data){
    if(data.ai_resp){
      var btn=document.getElementById('btn-ai-analyze');
      if(btn){btn.disabled=false;btn.innerText='\u73AF\u5883\u5206\u6790';btn.classList.remove('opacity-50','cursor-not-allowed');}
      appendChat('ai',data.ai_resp);
    }
    if(data.current){
      var c=data.current;
      document.getElementById('val-temp').innerText=(c.temp!=null)?c.temp.toFixed(1)+' \u00B0C':'-- \u00B0C';
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
        if(sys.rssi>=-50){q.innerText='\u6781\u597D';q.style.color='#16a34a';}
        else if(sys.rssi>=-70){q.innerText='\u826F\u597D';q.style.color='#2563eb';}
        else if(sys.rssi>=-80){q.innerText='\u4E00\u822C';q.style.color='#d97706';}
        else{q.innerText='\u8F83\u5DEE';q.style.color='#dc2626';}
      }
      if(sys.chip_rev)document.getElementById('val-chip-rev').innerText='Rev '+sys.chip_rev;
      if(sys.mac)document.getElementById('val-mac').innerText=sys.mac;
      if(sys.ip)document.getElementById('val-ip').innerText=sys.ip;
      if(sys.cpu_freq)document.getElementById('val-cpu-freq').innerText=sys.cpu_freq+'MHz';
      if(sys.cpu_cores)document.getElementById('val-cpu-cores').innerText=sys.cpu_cores+'\u6838';
      if(sys.uptime!=null)document.getElementById('val-uptime').innerText=fmtUptime(sys.uptime);
      if(sys.sdk_ver)document.getElementById('val-sdk').innerText=sys.sdk_ver;
      if(sys.fs_total){
        var fsPct=(sys.fs_used/sys.fs_total*100).toFixed(1);
        var si=document.getElementById('storage-info');
        if(si)si.innerText='\u5B58\u50A8: '+(sys.fs_used/1024).toFixed(1)+'KB / '+(sys.fs_total/1024).toFixed(0)+'KB ('+fsPct+'%)';
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
      mt.innerText=isAuto?'\u81EA\u52A8':'\u624B\u52A8';
      mt.style.color=isAuto?'#16a34a':'#ea580c';
      document.getElementById('mode-badge').innerText=isAuto?'\u81EA\u52A8\u6A21\u5F0F':'\u624B\u52A8\u6A21\u5F0F';
      document.getElementById('mode-badge').className='text-xs px-3 py-1 rounded-full font-medium '+(isAuto?'bg-green-50 text-green-600':'bg-orange-50 text-orange-600');
      ['pump','light','heater','fan'].forEach(function(dev){
        var el=document.getElementById('cb-'+dev);
        if(el){var on=(data.state[dev]===1);el.classList.toggle('on',on);el.classList.toggle('disabled',isAuto);}
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
    if(data.time){document.getElementById('val-time-sync').innerText='\u7CFB\u7EDF\u65F6\u95F4: '+data.time;document.getElementById('val-sys-time').innerText=data.time;}
    if(data.saveInterval){var si=document.getElementById('save-interval');if(si&&si!==document.activeElement)si.value=data.saveInterval;}
  }

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
    socket.send(JSON.stringify({action:'set_sched',fan_en:document.getElementById('sched-fan-en').checked,fan_start:document.getElementById('sched-fan-start').value||'00:00',fan_end:document.getElementById('sched-fan-end').value||'00:00',light_en:document.getElementById('sched-light-en').checked,light_start:document.getElementById('sched-light-start').value||'00:00',light_end:document.getElementById('sched-light-end').value||'00:00'}));
  };

  function appendChat(role,msg){
    var box=document.getElementById('ai-chat-box');
    var outer=document.createElement('div');
    outer.className=role==='user'?'flex justify-end pr-2 pl-8 mb-3':'flex justify-start pl-2 pr-8 mb-3';
    var inner=document.createElement('div');
    inner.className=role==='user'?'bg-blue-600 text-white p-3 rounded-2xl rounded-tr-none text-sm break-words':'bg-gray-50 border border-gray-100 p-3 rounded-2xl rounded-tl-none text-sm text-gray-700 leading-relaxed break-words whitespace-pre-wrap';
    inner.innerText=msg;outer.appendChild(inner);box.appendChild(outer);box.scrollTop=box.scrollHeight;
  }
  window.askAI=function(){
    if(socket&&socket.readyState===WebSocket.OPEN){
      var btn=document.getElementById('btn-ai-analyze');
      btn.disabled=true;btn.innerText='\u5206\u6790\u4E2D...';btn.classList.add('opacity-50','cursor-not-allowed');
      appendChat('user','\u8BF7\u6839\u636E\u5F53\u524D\u73AF\u5883\u6570\u636E\u5206\u6790\u5927\u68DA\u72B6\u6001\u5E76\u7ED9\u51FA\u5EFA\u8BAE\u3002');
      socket.send(JSON.stringify({action:'ai_analyze'}));
    }
  };
  window.sendAIChat=function(){
    var input=document.getElementById('ai-input');var q=input.value.trim();
    if(!q||!socket||socket.readyState!==WebSocket.OPEN)return;
    appendChat('user',q);socket.send(JSON.stringify({action:'ai_chat',question:q}));input.value='';
  };
  document.getElementById('ai-input')&&document.getElementById('ai-input').addEventListener('keypress',function(e){if(e.key==='Enter')sendAIChat();});

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
      var dd=data.data_dates||[];var ld=data.log_dates||[];
      var all={};dd.forEach(function(d){all[d]=true;});ld.forEach(function(d){all[d]=true;});
      var dates=Object.keys(all).sort().reverse();
      if(dates.length===0){sel.innerHTML='<option value="">\u6682\u65E0\u5F52\u6863</option>';}
      else{dates.forEach(function(d){var o=document.createElement('option');o.value=d;o.text=d.substring(0,4)+'-'+d.substring(4,6)+'-'+d.substring(6,8);sel.appendChild(o);});}
      if(data.total_bytes){
        var si=document.getElementById('storage-info');
        if(si)si.innerText='\u5B58\u50A8: '+(data.used_bytes/1024).toFixed(1)+'KB / '+(data.total_bytes/1024).toFixed(0)+'KB | \u5F52\u6863: '+dates.length+'\u5929';
      }
      // 填充导出日期选择器
      var es=document.getElementById('export-start'),ee=document.getElementById('export-end');
      if(es&&ee){es.innerHTML='';ee.innerHTML='';
        dates.forEach(function(d){var t=d.substring(0,4)+'-'+d.substring(4,6)+'-'+d.substring(6,8);
          var o1=document.createElement('option');o1.value=d;o1.text=t;es.appendChild(o1);
          var o2=document.createElement('option');o2.value=d;o2.text=t;ee.appendChild(o2);
        });
        if(dates.length>0)ee.value=dates[0];
      }
    }).catch(function(){});
  }
  setTimeout(loadAvailableDates,2000);
  // 加载记录粒度
  fetch('/api/interval').then(function(r){return r.json();}).then(function(d){
    var el=document.getElementById('save-interval');if(el&&d.interval)el.value=d.interval;
    var h=document.getElementById('interval-hint');
    if(h&&d.interval){var m=(d.interval/60).toFixed(1);h.innerText='\u5F53\u524D: '+d.interval+'\u79D2 ('+m+'\u5206\u949F)';}
  }).catch(function(){});
  window.saveSaveInterval=function(){
    var val=parseInt(document.getElementById('save-interval').value)||300;
    if(val<10)val=10;if(val>3600)val=3600;
    fetch('/api/interval',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({interval:val})}).then(function(r){return r.json();}).then(function(d){
      if(d.interval!=null){alert('\u8BB0\u5F55\u7C92\u5EA6\u5DF2\u8BBE\u4E3A '+d.interval+' \u79D2');
        var h=document.getElementById('interval-hint');if(h){var m=(d.interval/60).toFixed(1);h.innerText='\u5F53\u524D: '+d.interval+'\u79D2 ('+m+'\u5206\u949F)';}}
    }).catch(function(e){alert('\u4FDD\u5B58\u5931\u8D25: '+e.message);});
  };
  window.exportCSV=function(){
    var s=document.getElementById('export-start').value;
    var e=document.getElementById('export-end').value;
    if(!s||!e){alert('\u8BF7\u9009\u62E9\u65E5\u671F\u8303\u56F4');return;}
    if(s>e){var tmp=s;s=e;e=tmp;}
    window.open('/api/export?start='+s+'&end='+e,'_blank');
  };

  window.loadHistoryData=function(){
    var date=document.getElementById('hist-date-select').value;if(!date)return;
    fetch('/api/history?date='+date).then(function(r){return r.json();}).then(function(data){
      if(data.error){alert(data.error);return;}
      document.getElementById('hist-charts-wrap').style.display='';
      document.getElementById('hist-stats-wrap').style.display='';
      document.getElementById('hist-log-wrap').style.display='none';
      initHistCharts();
      var dl=date.substring(0,4)+'-'+date.substring(4,6)+'-'+date.substring(6,8);
      histChartTH.setOption(makeChartOpt(dl+' \u6E29\u6E7F\u5EA6',['\u6E29\u5EA6(\u00B0C)','\u6E7F\u5EA6(%)'],['#ef4444','#3b82f6'],['\u00B0C','%']));
      histChartTH.setOption({xAxis:{data:data.time||[]},series:[{data:data.temp||[]},{data:data.hum||[]}]});
      histChartLS.setOption(makeChartOpt(dl+' \u5149\u7167\u00B7\u571F\u58E4',['\u5149\u7167(lx)','\u571F\u58E4(%)'],['#f97316','#22c55e'],['lx','%']));
      histChartLS.setOption({xAxis:{data:data.time||[]},series:[{data:data.lux||[]},{data:data.soil||[]}]});
      histChartAQ.setOption(makeChartOpt(dl+' \u7A7A\u6C14\u8D28\u91CF',['eCO2(ppm)','TVOC(ppb)'],['#14b8a6','#a855f7'],['ppm','ppb'],400));
      histChartAQ.setOption({xAxis:{data:data.time||[]},series:[{data:data.eco2||[]},{data:data.tvoc||[]}]});
      var metrics=[{key:'temp',label:'\u{1F321}\uFE0F \u6E29\u5EA6',unit:'\u00B0C',dec:1},{key:'hum',label:'\u{1F4A7} \u6E7F\u5EA6',unit:'%',dec:1},{key:'lux',label:'\u2600\uFE0F \u5149\u7167',unit:'lx',dec:0},{key:'soil',label:'\u{1F331} \u571F\u58E4',unit:'%',dec:0},{key:'eco2',label:'\u2601\uFE0F eCO2',unit:'ppm',dec:0},{key:'tvoc',label:'\u{1F9EA} TVOC',unit:'ppb',dec:0}];
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
    }).catch(function(e){alert('\u67E5\u8BE2\u5931\u8D25: '+e.message);});
  };
  window.loadHistoryLog=function(){
    var date=document.getElementById('hist-date-select').value;if(!date)return;
    fetch('/api/log?date='+date).then(function(r){return r.json();}).then(function(data){
      if(data.error){alert(data.error);return;}
      document.getElementById('hist-charts-wrap').style.display='none';
      document.getElementById('hist-stats-wrap').style.display='none';
      document.getElementById('hist-log-wrap').style.display='';
      var list=document.getElementById('hist-log-list');
      if(!data.entries||data.entries.length===0){list.innerHTML='<div class="text-gray-400">\u5F53\u65E5\u65E0\u8BB0\u5F55</div>';}
      else{list.innerHTML=data.entries.map(function(e){return'<div class="py-1 border-b border-slate-200">'+e.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')+'</div>';}).join('');}
    }).catch(function(e){alert('\u67E5\u8BE2\u5931\u8D25: '+e.message);});
  };

  function loadQQBotConfig(){
    fetch('/api/qqbot/config').then(function(r){return r.json();}).then(function(d){
      document.getElementById('qqbot-appid').value=d.appId||'';
      document.getElementById('qqbot-groupid').value=d.userOpenId||'';
      document.getElementById('qqbot-enabled').checked=d.enabled||false;
      var st=document.getElementById('qqbot-status');
      if(!d.appId){st.innerText='\u672A\u914D\u7F6E';st.style.color='#9ca3af';}
      else if(d.gatewayReady){st.innerText='Gateway \u5C31\u7EEA';st.style.color='#16a34a';}
      else if(d.tokenValid){st.innerText='Token \u6709\u6548';st.style.color='#2563eb';}
      else if(d.enabled){st.innerText='\u5DF2\u542F\u7528';st.style.color='#2563eb';}
      else{st.innerText='\u5DF2\u7981\u7528';st.style.color='#ea580c';}
      var dots=[document.getElementById('gw-status-dot'),document.getElementById('gw-dot-side')];
      var gwt=document.getElementById('gw-status-text');
      var gws=document.getElementById('gw-status-side');
      if(d.gatewayReady){
        dots.forEach(function(x){if(x)x.style.background='#22c55e';});
        if(gwt)gwt.innerText='\u5DF2\u8FDE\u63A5';if(gws)gws.innerText='GW \u5728\u7EBF';
      }else if(d.gatewayConnected){
        dots.forEach(function(x){if(x)x.style.background='#f59e0b';});
        if(gwt)gwt.innerText='\u8FDE\u63A5\u4E2D...';if(gws)gws.innerText='GW \u8FDE\u63A5\u4E2D';
      }else if(d.enabled){
        dots.forEach(function(x){if(x)x.style.background='#ef4444';});
        if(gwt)gwt.innerText='\u672A\u8FDE\u63A5';if(gws)gws.innerText='GW \u79BB\u7EBF';
      }else{
        dots.forEach(function(x){if(x)x.style.background='#9ca3af';});
        if(gwt)gwt.innerText='\u672A\u542F\u7528';if(gws)gws.innerText='GW \u672A\u542F\u7528';
      }
    }).catch(function(){});
  }
  setTimeout(loadQQBotConfig,3000);
  setInterval(loadQQBotConfig,15000);
  window.saveQQBotConfig=function(){
    var body={appId:document.getElementById('qqbot-appid').value,userOpenId:document.getElementById('qqbot-groupid').value,enabled:document.getElementById('qqbot-enabled').checked};
    var sec=document.getElementById('qqbot-secret').value;if(sec)body.appSecret=sec;
    fetch('/api/qqbot/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}).then(function(r){return r.json();}).then(function(d){
      if(d.ok){alert('\u914D\u7F6E\u5DF2\u4FDD\u5B58');loadQQBotConfig();}
    }).catch(function(e){alert('\u4FDD\u5B58\u5931\u8D25: '+e.message);});
  };
  window.testQQBot=function(){
    fetch('/api/qqbot/test').then(function(r){return r.json();}).then(function(d){
      alert(d.ok?'\u6D4B\u8BD5\u6D88\u606F\u53D1\u9001\u6210\u529F\uFF01':('\u5931\u8D25: '+(d.error||'\u672A\u77E5\u9519\u8BEF')));
    }).catch(function(e){alert('\u8BF7\u6C42\u5931\u8D25: '+e.message);});
  };
  window.addEventListener('resize',function(){chartTH.resize();chartLS.resize();chartAQ.resize();if(histChartTH){histChartTH.resize();histChartLS.resize();histChartAQ.resize();}});
})();
</script>
</body>
</html>
)rawliteral";
