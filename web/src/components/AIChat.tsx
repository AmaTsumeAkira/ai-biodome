import { useState, useRef, useEffect, useCallback } from 'react';
import { FloatButton, Drawer, Input, Button, Flex, Typography, Space } from 'antd';
import { CommentOutlined, ExperimentOutlined, SendOutlined, CloseOutlined } from '@ant-design/icons';
import { useApp } from '../context/AppContext';

const { Text } = Typography;

export default function AIChat() {
  const { wsSend, chatMessages, addChat } = useApp();
  const [open, setOpen] = useState(false);
  const [input, setInput] = useState('');
  const [analyzing, setAnalyzing] = useState(false);
  const bodyRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (bodyRef.current) bodyRef.current.scrollTop = bodyRef.current.scrollHeight;
  }, [chatMessages]);

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

  return (
    <>
      <FloatButton
        icon={<CommentOutlined />}
        type="primary"
        tooltip="🧠 AI 智能助手"
        onClick={() => setOpen(true)}
        style={{ insetInlineEnd: 24, insetBlockEnd: 80 }}
      />

      <Drawer
        title={
          <Flex align="center" gap={8}>
            <span style={{ fontSize: 18 }}>🧠</span>
            <Text strong>AI 智能助手</Text>
          </Flex>
        }
        placement="right"
        width={360}
        open={open}
        onClose={() => setOpen(false)}
        closeIcon={<CloseOutlined />}
        extra={
          <Button
            size="small"
            type="primary"
            icon={<ExperimentOutlined />}
            loading={analyzing}
            onClick={askAnalyze}
          >
            环境分析
          </Button>
        }
        styles={{ body: { padding: 0, display: 'flex', flexDirection: 'column' } }}
      >
        {/* Message area */}
        <div ref={bodyRef} style={{ flex: 1, overflow: 'auto', padding: 16 }}>
          {chatMessages.length === 0 && (
            <Text type="secondary" style={{ display: 'block', textAlign: 'center', marginTop: 40 }}>
              向 AI 提问，或点击"环境分析"
            </Text>
          )}
          {chatMessages.map((m, i) => (
            <div
              key={i}
              style={{
                display: 'flex',
                justifyContent: m.role === 'user' ? 'flex-end' : 'flex-start',
                marginBottom: 12,
                paddingLeft: m.role === 'user' ? 40 : 0,
                paddingRight: m.role === 'user' ? 0 : 40,
              }}
            >
              <div
                className={m.role === 'user' ? 'chat-bubble-user' : 'chat-bubble-ai'}
              >
                {m.text}
              </div>
            </div>
          ))}
        </div>

        {/* Input area */}
        <div style={{ padding: '12px 16px', borderTop: '1px solid #f0f0f0' }}>
          <Space.Compact style={{ width: '100%' }}>
            <Input
              placeholder="输入你的问题..."
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onPressEnter={sendChat}
            />
            <Button type="primary" icon={<SendOutlined />} onClick={sendChat}>发送</Button>
          </Space.Compact>
        </div>
      </Drawer>
    </>
  );
}
