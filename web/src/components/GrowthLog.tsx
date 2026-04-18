import { useState, useEffect, useCallback } from 'react';
import { Card, Button, Input, Select, Timeline, Empty, Typography, Flex, Modal, App, Tag, Popconfirm } from 'antd';
import { PlusOutlined, DeleteOutlined, ExperimentOutlined } from '@ant-design/icons';
import { useApp } from '../context/AppContext';
import type { GrowLogEntry } from '../types';

const { Title, Text, Paragraph } = Typography;
const { TextArea } = Input;

const STAGES = [
  { value: 'seed', label: '\u{1F331} \u64AD\u79CD', color: '#8b5cf6' },
  { value: 'sprout', label: '\u{1F33F} \u53D1\u82BD', color: '#22c55e' },
  { value: 'grow', label: '\u{1F343} \u751F\u957F', color: '#10b981' },
  { value: 'flower', label: '\u{1F33C} \u5F00\u82B1', color: '#f59e0b' },
  { value: 'fruit', label: '\u{1F345} \u7ED3\u679C', color: '#ef4444' },
  { value: 'harvest', label: '\u{1F33E} \u6536\u83B7', color: '#f97316' },
  { value: 'maintain', label: '\u{1F527} \u7EF4\u62A4', color: '#64748b' },
  { value: 'note', label: '\u{1F4DD} \u7B14\u8BB0', color: '#0ea5e9' },
];

function getStageInfo(stage: string) {
  return STAGES.find((s) => s.value === stage) || STAGES[7];
}

export default function GrowthLog() {
  const { message: msg } = App.useApp();
  const { state } = useApp();
  const [entries, setEntries] = useState<GrowLogEntry[]>([]);
  const [loading, setLoading] = useState(false);
  const [modalOpen, setModalOpen] = useState(false);
  const [newStage, setNewStage] = useState('note');
  const [newNote, setNewNote] = useState('');
  const [addEnv, setAddEnv] = useState(true);

  const fetchEntries = useCallback(async () => {
    setLoading(true);
    try {
      const res = await fetch('/api/growlog');
      const data = await res.json();
      setEntries(data.entries || []);
    } catch {
      /* offline */
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => { fetchEntries(); }, [fetchEntries]);

  const addEntry = async () => {
    if (!newNote.trim()) { msg.warning('\u8BF7\u8F93\u5165\u5185\u5BB9'); return; }
    try {
      const body: Record<string, unknown> = { stage: newStage, note: newNote.trim() };
      if (addEnv && state.current.temp != null) {
        body.temp = +(state.current.temp.toFixed(1));
        body.hum = state.current.hum != null ? +(state.current.hum.toFixed(1)) : undefined;
        body.soil = state.current.soil;
      }
      await fetch('/api/growlog', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
      });
      msg.success('\u8BB0\u5F55\u5DF2\u6DFB\u52A0');
      setNewNote('');
      setModalOpen(false);
      fetchEntries();
    } catch (e) {
      msg.error('\u6DFB\u52A0\u5931\u8D25: ' + (e as Error).message);
    }
  };

  const deleteEntry = async (id: number) => {
    try {
      await fetch(`/api/growlog?id=${id}`, { method: 'DELETE' });
      msg.success('\u5DF2\u5220\u9664');
      fetchEntries();
    } catch (e) {
      msg.error('\u5220\u9664\u5931\u8D25: ' + (e as Error).message);
    }
  };

  const dayCount = entries.length > 0
    ? Math.ceil((Date.now() - new Date(entries[entries.length - 1].date).getTime()) / 86400000)
    : 0;

  return (
    <>
      <div className="page-header">
        <Flex justify="space-between" align="center">
          <div>
            <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>
              {'\u{1F331}'} {'\u751F\u957F\u65E5\u5FD7'}
            </Title>
            <Text type="secondary" style={{ fontSize: 11 }}>
              {entries.length > 0 ? `\u5DF2\u8BB0\u5F55 ${entries.length} \u6761 \u00B7 \u7B2C ${dayCount} \u5929` : '\u6682\u65E0\u8BB0\u5F55'}
            </Text>
          </div>
          <Button type="primary" icon={<PlusOutlined />} onClick={() => setModalOpen(true)}>
            {'\u65B0\u589E\u8BB0\u5F55'}
          </Button>
        </Flex>
      </div>

      {entries.length === 0 ? (
        <Card className="ind-card">
          <Empty description={'\u8FD8\u6CA1\u6709\u79CD\u690D\u8BB0\u5F55\uFF0C\u70B9\u51FB\u201C\u65B0\u589E\u8BB0\u5F55\u201D\u5F00\u59CB\u5427\uFF01'} />
        </Card>
      ) : (
        <Card className="ind-card" style={{ maxHeight: 'calc(100vh - 200px)', overflow: 'auto' }}>
          <Timeline
            mode="left"
            items={entries.map((e) => {
              const si = getStageInfo(e.stage);
              return {
                key: e.id,
                color: si.color,
                dot: <ExperimentOutlined style={{ fontSize: 14, color: si.color }} />,
                children: (
                  <Card
                    size="small"
                    style={{ borderLeft: `3px solid ${si.color}`, boxShadow: '0 1px 4px rgba(0,0,0,0.06)' }}
                  >
                    <Flex justify="space-between" align="start">
                      <div style={{ flex: 1 }}>
                        <Flex align="center" gap={6} style={{ marginBottom: 4 }}>
                          <Tag color={si.color} style={{ fontSize: 11, fontWeight: 600 }}>{si.label}</Tag>
                          <Text type="secondary" style={{ fontSize: 11 }}>{e.date}</Text>
                        </Flex>
                        <Paragraph style={{ margin: 0, fontSize: 13, whiteSpace: 'pre-wrap' }}>{e.note}</Paragraph>
                        {(e.temp != null || e.soil != null) && (
                          <Flex gap={8} style={{ marginTop: 4 }}>
                            {e.temp != null && <Tag style={{ fontSize: 10 }}>{'\u{1F321}\uFE0F'} {e.temp}{'\u00B0C'}</Tag>}
                            {e.hum != null && <Tag style={{ fontSize: 10 }}>{'\u{1F4A7}'} {e.hum}%</Tag>}
                            {e.soil != null && <Tag style={{ fontSize: 10 }}>{'\u{1F331}'} {e.soil}%</Tag>}
                          </Flex>
                        )}
                      </div>
                      <Popconfirm title={'\u786E\u8BA4\u5220\u9664\uFF1F'} onConfirm={() => deleteEntry(e.id)}>
                        <Button type="text" size="small" icon={<DeleteOutlined />} danger />
                      </Popconfirm>
                    </Flex>
                  </Card>
                ),
              };
            })}
          />
        </Card>
      )}

      <Modal
        title={'\u{1F4DD} \u65B0\u589E\u79CD\u690D\u8BB0\u5F55'}
        open={modalOpen}
        onOk={addEntry}
        onCancel={() => setModalOpen(false)}
        okText={'\u6DFB\u52A0'}
        cancelText={'\u53D6\u6D88'}
        confirmLoading={loading}
      >
        <Flex vertical gap={12}>
          <div>
            <Text style={{ fontSize: 12, fontWeight: 600 }}>{'\u751F\u957F\u9636\u6BB5'}</Text>
            <Select
              value={newStage}
              onChange={setNewStage}
              options={STAGES}
              style={{ width: '100%', marginTop: 4 }}
            />
          </div>
          <div>
            <Text style={{ fontSize: 12, fontWeight: 600 }}>{'\u8BB0\u5F55\u5185\u5BB9'}</Text>
            <TextArea
              value={newNote}
              onChange={(e) => setNewNote(e.target.value)}
              placeholder={'\u8BB0\u5F55\u89C2\u5BDF\u5230\u7684\u53D8\u5316\u3001\u64CD\u4F5C\u3001\u6CE8\u610F\u4E8B\u9879...'}
              rows={4}
              maxLength={500}
              showCount
              style={{ marginTop: 4 }}
            />
          </div>
          <Flex align="center" gap={8}>
            <input type="checkbox" checked={addEnv} onChange={(e) => setAddEnv(e.target.checked)} />
            <Text style={{ fontSize: 12 }}>{'\u9644\u52A0\u5F53\u524D\u73AF\u5883\u6570\u636E'} ({'\u6E29\u5EA6'}{state.current.temp?.toFixed(1) ?? '--'}{'\u00B0C'}, {'\u571F\u58E4'}{state.current.soil ?? '--'}%)</Text>
          </Flex>
        </Flex>
      </Modal>
    </>
  );
}
