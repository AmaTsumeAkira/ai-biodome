import { useState, useRef, useEffect, useCallback } from 'react';
import { useApp } from '../context/AppContext';

export default function AIChat() {
  const { wsSend, chatMessages, addChat } = useApp();
  const [open, setOpen] = useState(false);
  const [input, setInput] = useState('');
  const [analyzing, setAnalyzing] = useState(false);
  const bodyRef = useRef<HTMLDivElement>(null);

  // Scroll to bottom when new messages
  useEffect(() => {
    if (bodyRef.current) bodyRef.current.scrollTop = bodyRef.current.scrollHeight;
  }, [chatMessages]);

  // Reset analyzing state when AI response comes
  useEffect(() => {
    if (analyzing && chatMessages.length > 0 && chatMessages[chatMessages.length - 1].role === 'ai') {
      setAnalyzing(false);
    }
  }, [chatMessages, analyzing]);

  const askAnalyze = useCallback(() => {
    setAnalyzing(true);
    addChat({ role: 'user', text: '请根据当前环境数据分析大棚状态并给出建议。' });
    wsSend({ action: 'ai_analyze' });
  }, [wsSend, addChat]);

  const sendChat = useCallback(() => {
    const q = input.trim();
    if (!q) return;
    addChat({ role: 'user', text: q });
    wsSend({ action: 'ai_chat', question: q });
    setInput('');
  }, [input, wsSend, addChat]);

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') sendChat();
  };

  return (
    <>
      {/* Floating button */}
      <button className="ai-float-btn" onClick={() => setOpen(!open)}>💬</button>

      {/* Drawer */}
      <div className={`ai-drawer ${open ? 'open' : ''}`}>
        <div className="ai-drawer-head">
          <div className="flex items-center gap-2">
            <span className="text-lg">🧠</span>
            <span className="font-bold text-sm">AI 智能助手</span>
          </div>
          <div className="flex items-center gap-2">
            <button
              onClick={askAnalyze}
              disabled={analyzing}
              className={`px-3 py-1 bg-white/20 hover:bg-white/30 rounded-lg text-xs font-medium transition-colors ${analyzing ? 'opacity-50 cursor-not-allowed' : ''}`}
            >
              {analyzing ? '分析中...' : '环境分析'}
            </button>
            <button
              onClick={() => setOpen(false)}
              className="text-white/80 hover:text-white text-lg"
              style={{ background: 'none', border: 'none', cursor: 'pointer' }}
            >✕</button>
          </div>
        </div>

        <div className="ai-drawer-body" ref={bodyRef}>
          {chatMessages.map((msg, i) => (
            <div key={i} className={msg.role === 'user' ? 'flex justify-end pr-2 pl-8 mb-3' : 'flex justify-start pl-2 pr-8 mb-3'}>
              <div className={
                msg.role === 'user'
                  ? 'bg-blue-600 text-white p-3 rounded-2xl rounded-tr-none text-sm break-words'
                  : 'bg-gray-50 border border-gray-100 p-3 rounded-2xl rounded-tl-none text-sm text-gray-700 leading-relaxed break-words whitespace-pre-wrap'
              }>
                {msg.text}
              </div>
            </div>
          ))}
        </div>

        <div className="ai-drawer-foot">
          <input
            type="text"
            placeholder="输入你的问题..."
            className="flex-1 px-3 py-2 text-sm rounded-lg border focus:ring-2 focus:ring-blue-400 outline-none"
            value={input}
            onChange={(e) => setInput(e.target.value)}
            onKeyDown={handleKeyDown}
          />
          <button onClick={sendChat} className="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg text-sm font-medium transition-colors">
            发送
          </button>
        </div>
      </div>
    </>
  );
}
