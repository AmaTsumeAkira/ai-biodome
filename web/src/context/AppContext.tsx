import { createContext, useContext, useReducer, useCallback, useEffect, useRef, type ReactNode, type Dispatch } from 'react';
import type { AppState, WSMessage, ChatMessage } from '../types';

const initialState: AppState = {
  current: { temp: null, hum: null, lux: null, soil: null, eco2: null, tvoc: null },
  state: { mode: 'auto', pump: 0, light: 0, heater: 0, fan: 0 },
  system: null,
  sensors: { bh1750: false, sht40: false, sgp30: false, soil: true },
  history: { time: [], temp: [], hum: [], lux: [], soil: [], eco2: [], tvoc: [] },
  sched: { fan_en: false, fan_start: '00:00', fan_end: '00:00', light_en: false, light_start: '00:00', light_end: '00:00' },
  time: '',
  saveInterval: 300,
  wsStatus: 'disconnected',
};

type Action =
  | { type: 'WS_MESSAGE'; payload: WSMessage }
  | { type: 'WS_STATUS'; payload: AppState['wsStatus'] }
  | { type: 'AI_RESP'; payload: string };

function reducer(s: AppState, a: Action): AppState {
  switch (a.type) {
    case 'WS_STATUS':
      return { ...s, wsStatus: a.payload };
    case 'WS_MESSAGE': {
      const m = a.payload;
      const next = { ...s };
      if (m.current) next.current = m.current;
      if (m.state) next.state = m.state;
      if (m.system) next.system = m.system;
      if (m.sensors) next.sensors = m.sensors;
      if (m.history && m.history.time?.length) next.history = m.history;
      if (m.sched) next.sched = m.sched;
      if (m.time) next.time = m.time;
      if (m.saveInterval) next.saveInterval = m.saveInterval;
      return next;
    }
    default:
      return s;
  }
}

interface AppContextValue {
  state: AppState;
  dispatch: Dispatch<Action>;
  wsSend: (data: object) => void;
  chatMessages: ChatMessage[];
  addChat: (msg: ChatMessage) => void;
}

const AppContext = createContext<AppContextValue | null>(null);

export function useApp() {
  const ctx = useContext(AppContext);
  if (!ctx) throw new Error('useApp must be used within AppProvider');
  return ctx;
}

export function AppProvider({ children }: { children: ReactNode }) {
  const [state, dispatch] = useReducer(reducer, initialState);
  const socketRef = useRef<WebSocket | null>(null);
  const reconnectRef = useRef<ReturnType<typeof setTimeout>>(undefined);
  const chatRef = useRef<ChatMessage[]>([
    { role: 'ai', text: '你好！我是 AI 助手，可以帮你分析大棚环境数据。点击"环境分析"或直接提问。' },
  ]);
  const [chatMessages, setChatMessages] = useReducer(
    (_: ChatMessage[], a: ChatMessage[]) => a,
    chatRef.current,
  );

  const addChat = useCallback((msg: ChatMessage) => {
    chatRef.current = [...chatRef.current, msg];
    setChatMessages(chatRef.current);
  }, []);

  const connect = useCallback(() => {
    if (reconnectRef.current) clearTimeout(reconnectRef.current);
    const ws = new WebSocket(`ws://${location.hostname}:81/`);
    socketRef.current = ws;
    dispatch({ type: 'WS_STATUS', payload: 'connecting' });

    ws.onopen = () => dispatch({ type: 'WS_STATUS', payload: 'connected' });
    ws.onclose = () => {
      dispatch({ type: 'WS_STATUS', payload: 'disconnected' });
      reconnectRef.current = setTimeout(connect, 3000);
    };
    ws.onerror = () => dispatch({ type: 'WS_STATUS', payload: 'disconnected' });
    ws.onmessage = (e) => {
      try {
        const data: WSMessage = JSON.parse(e.data);
        if (data.ai_resp) addChat({ role: 'ai', text: data.ai_resp });
        dispatch({ type: 'WS_MESSAGE', payload: data });
      } catch { /* ignore */ }
    };
  }, [addChat]);

  useEffect(() => {
    connect();
    const onVis = () => {
      if (!document.hidden && (!socketRef.current || socketRef.current.readyState === WebSocket.CLOSED)) connect();
    };
    document.addEventListener('visibilitychange', onVis);
    return () => {
      document.removeEventListener('visibilitychange', onVis);
      socketRef.current?.close();
      if (reconnectRef.current) clearTimeout(reconnectRef.current);
    };
  }, [connect]);

  const wsSend = useCallback((data: object) => {
    if (socketRef.current?.readyState === WebSocket.OPEN) {
      socketRef.current.send(JSON.stringify(data));
    }
  }, []);

  return (
    <AppContext.Provider value={{ state, dispatch, wsSend, chatMessages, addChat }}>
      {children}
    </AppContext.Provider>
  );
}
