import { useCallback } from 'react';
import { Card, Row, Col, Switch, Typography, Flex, TimePicker, Tag } from 'antd';
import dayjs from 'dayjs';
import { useApp } from '../context/AppContext';

const { Title, Text } = Typography;

const DEVICES = [
  { id: 'pump', label: '💧 水泵', desc: '土壤灦溉', color: '#0ea5e9' },
  { id: 'light', label: '💡 补光灯', desc: '植物补光', color: '#f59e0b' },
  { id: 'heater', label: '🔥 加热垫', desc: '温度调节', color: '#ef4444' },
  { id: 'fan', label: '🌬️ 排风扇', desc: '通风换气', color: '#06b6d4' },
] as const;

export default function Control() {
  const { state, wsSend } = useApp();
  const { state: devState, sched } = state;
  const isAuto = devState.mode === 'auto';

  const toggleMode = () => {
    wsSend({ action: 'set_mode', mode: isAuto ? 'manual' : 'auto' });
  };

  const toggleDevice = (dev: string, checked: boolean) => {
    if (isAuto) return;
    wsSend({ action: 'set_device', device: dev, state: checked ? 1 : 0 });
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
    <>
      <div className="page-header">
        <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>🎮 设备控制</Title>
      </div>

      {/* Mode toggle */}
      <Card style={{ marginBottom: 16, borderLeft: '3px solid #475569', boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
        <Flex justify="space-between" align="center">
          <div>
            <Text strong style={{ fontSize: 14 }}>运行模式</Text>
            <br />
            <Text type="secondary" style={{ fontSize: 11 }}>自动模式下设备根据传感器数据自动调节</Text>
          </div>
          <Flex align="center" gap={10}>
            <Tag color={isAuto ? 'success' : 'warning'} style={{ fontSize: 11, fontWeight: 600, padding: '0 8px' }}>
              {isAuto ? 'AUTO' : 'MANUAL'}
            </Tag>
            <Switch
              checked={isAuto}
              onChange={toggleMode}
              checkedChildren="AUTO"
              unCheckedChildren="MAN"
            />
          </Flex>
        </Flex>
      </Card>

      {/* Device cards */}
      <Row gutter={[8, 8]} style={{ marginBottom: 16 }}>
        {DEVICES.map((d) => {
          const isOn = (devState[d.id] as number) === 1;
          return (
            <Col xs={24} sm={12} key={d.id}>
              <Card
                size="small"
                className="device-card"
                style={{ borderLeft: `3px solid ${d.color}`, boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}
              >
                <Flex justify="space-between" align="center">
                  <Flex align="center" gap={10}>
                    <span className="ind-led" style={{ color: isOn ? d.color : '#94a3b8', width: 10, height: 10 }} />
                    <div>
                      <Text strong style={{ fontSize: 13 }}>{d.label}</Text>
                      <br />
                      <Text type="secondary" style={{ fontSize: 10, letterSpacing: 0.3 }}>{d.desc}</Text>
                    </div>
                  </Flex>
                  <Switch
                    checked={isOn}
                    disabled={isAuto}
                    onChange={(checked) => toggleDevice(d.id, checked)}
                    checkedChildren="ON"
                    unCheckedChildren="OFF"
                  />
                </Flex>
              </Card>
            </Col>
          );
        })}
      </Row>

      {/* Schedule */}
      <Card title="⏰ 定时任务" className="ind-card" style={{ boxShadow: '0 2px 8px rgba(0,0,0,0.04)' }}>
        <Row gutter={[12, 12]}>
          <Col xs={24} md={12}>
            <Card
              size="small"
              type="inner"
              style={{ borderLeft: '3px solid #06b6d4' }}
              title={
                <Flex justify="space-between" align="center">
                  <Text style={{ fontSize: 12, fontWeight: 600 }}>定时通风 (FAN)</Text>
                  <Switch
                    size="small"
                    checked={sched.fan_en}
                    onChange={(checked) => sendSchedule({ fan_en: checked })}
                  />
                </Flex>
              }
            >
              <Flex align="center" gap={8}>
                <TimePicker
                  value={sched.fan_start ? dayjs(sched.fan_start, 'HH:mm') : null}
                  format="HH:mm"
                  onChange={(_, timeStr) => sendSchedule({ fan_start: (typeof timeStr === 'string' ? timeStr : timeStr?.[0]) || '00:00' })}
                  style={{ width: 100 }}
                  placeholder="START"
                  size="small"
                />
                <Text type="secondary" style={{ fontSize: 11 }}>→</Text>
                <TimePicker
                  value={sched.fan_end ? dayjs(sched.fan_end, 'HH:mm') : null}
                  format="HH:mm"
                  onChange={(_, timeStr) => sendSchedule({ fan_end: (typeof timeStr === 'string' ? timeStr : timeStr?.[0]) || '00:00' })}
                  style={{ width: 100 }}
                  placeholder="END"
                  size="small"
                />
              </Flex>
            </Card>
          </Col>
          <Col xs={24} md={12}>
            <Card
              size="small"
              type="inner"
              style={{ borderLeft: '3px solid #f59e0b' }}
              title={
                <Flex justify="space-between" align="center">
                  <Text style={{ fontSize: 12, fontWeight: 600 }}>定时补光 (LIGHT)</Text>
                  <Switch
                    size="small"
                    checked={sched.light_en}
                    onChange={(checked) => sendSchedule({ light_en: checked })}
                  />
                </Flex>
              }
            >
              <Flex align="center" gap={8}>
                <TimePicker
                  value={sched.light_start ? dayjs(sched.light_start, 'HH:mm') : null}
                  format="HH:mm"
                  onChange={(_, timeStr) => sendSchedule({ light_start: (typeof timeStr === 'string' ? timeStr : timeStr?.[0]) || '00:00' })}
                  style={{ width: 100 }}
                  placeholder="START"
                  size="small"
                />
                <Text type="secondary" style={{ fontSize: 11 }}>→</Text>
                <TimePicker
                  value={sched.light_end ? dayjs(sched.light_end, 'HH:mm') : null}
                  format="HH:mm"
                  onChange={(_, timeStr) => sendSchedule({ light_end: (typeof timeStr === 'string' ? timeStr : timeStr?.[0]) || '00:00' })}
                  style={{ width: 100 }}
                  placeholder="END"
                  size="small"
                />
              </Flex>
            </Card>
          </Col>
        </Row>
      </Card>
    </>
  );
}
