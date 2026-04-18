export interface SensorData {
  temp: number | null;
  hum: number | null;
  lux: number | null;
  soil: number | null;
  eco2: number | null;
  tvoc: number | null;
}

export interface DeviceState {
  mode: 'auto' | 'manual';
  pump: number;
  light: number;
  heater: number;
  fan: number;
}

export interface SystemInfo {
  heap_total: number;
  heap_free: number;
  chip_rev: number;
  cpu_cores: number;
  mac: string;
  sdk_ver: string;
  rssi: number;
  ip: string;
  cpu_freq: number;
  uptime: number;
  psram: boolean;
  fs_total?: number;
  fs_used?: number;
}

export interface SensorStatus {
  bh1750: boolean;
  sht40: boolean;
  sgp30: boolean;
  soil: boolean;
}

export interface Schedule {
  fan_en: boolean;
  fan_start: string;
  fan_end: string;
  light_en: boolean;
  light_start: string;
  light_end: string;
}

export interface HistoryData {
  time: string[];
  temp: number[];
  hum: number[];
  lux: number[];
  soil: number[];
  eco2: number[];
  tvoc: number[];
}

export interface WSMessage {
  current?: SensorData;
  state?: DeviceState;
  system?: SystemInfo;
  sensors?: SensorStatus;
  history?: HistoryData;
  sched?: Schedule;
  time?: string;
  saveInterval?: number;
  ai_resp?: string;
}

export interface AppState {
  current: SensorData;
  state: DeviceState;
  system: SystemInfo | null;
  sensors: SensorStatus;
  history: HistoryData;
  sched: Schedule;
  time: string;
  saveInterval: number;
  wsStatus: 'connected' | 'connecting' | 'disconnected';
}

export type Tab = 'dashboard' | 'twin' | 'control' | 'history' | 'qqbot' | 'system';

export interface ChatMessage {
  role: 'user' | 'ai';
  text: string;
}
