import { useState, useCallback } from 'react';
import { Card, Button, Spin, Typography, Flex, Tag, Empty, App, Progress, Alert } from 'antd';
import { ThunderboltOutlined, FileTextOutlined, ReloadOutlined } from '@ant-design/icons';
import { useApp } from '../context/AppContext';

const { Title, Text, Paragraph } = Typography;

interface ReportData {
  summary: string;
  score: number;
  suggestions: string[];
  timestamp: string;
}

function scoreColor(s: number) {
  if (s >= 80) return '#22c55e';
  if (s >= 60) return '#f59e0b';
  return '#ef4444';
}

function scoreLabel(s: number) {
  if (s >= 90) return '\u{1F31F} \u4F18\u79C0';
  if (s >= 80) return '\u2705 \u826F\u597D';
  if (s >= 60) return '\u26A0\uFE0F \u4E00\u822C';
  return '\u274C \u8F83\u5DEE';
}

export default function SmartReport() {
  const { message: msg } = App.useApp();
  const { state } = useApp();
  const [report, setReport] = useState<ReportData | null>(null);
  const [loading, setLoading] = useState(false);
  const [history, setHistory] = useState<ReportData[]>([]);
  const [aiConfigured, setAiConfigured] = useState(true);

  useState(() => {
    fetch('/api/ai/config')
      .then((r) => r.json())
      .then((d) => setAiConfigured(d.configured ?? false))
      .catch(() => {});
  });

  const generateReport = useCallback(async () => {
    if (!aiConfigured) {
      msg.warning('\u8BF7\u5148\u5728\u201C\u7CFB\u7EDF\u4FE1\u606F\u201D\u9875\u9762\u914D\u7F6E AI API Key');
      return;
    }
    setLoading(true);
    try {
      const res = await fetch('/api/report/generate', { method: 'POST' });
      const data = await res.json();
      if (data.error) {
        msg.error(data.error);
        return;
      }
      const parsed: ReportData = {
        summary: data.summary || '',
        score: data.score ?? 0,
        suggestions: data.suggestions || [],
        timestamp: data.timestamp || new Date().toLocaleString(),
      };
      setReport(parsed);
      setHistory((prev) => [parsed, ...prev].slice(0, 5));
    } catch (e) {
      msg.error('\u62A5\u544A\u751F\u6210\u5931\u8D25: ' + (e as Error).message);
    } finally {
      setLoading(false);
    }
  }, [msg]);

  const loadHistory = useCallback(async () => {
    try {
      const res = await fetch('/api/report/history');
      const data = await res.json();
      if (data.reports) setHistory(data.reports);
    } catch { /* offline */ }
  }, []);

  useState(() => { loadHistory(); });

  const { current } = state;

  return (
    <>
      <div className="page-header">
        <Flex justify="space-between" align="center">
          <div>
            <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>
              {'\u{1F4C8}'} {'\u667A\u80FD\u62A5\u544A'}
            </Title>
            <Text type="secondary" style={{ fontSize: 11 }}>
              {'\u57FA\u4E8E AI \u5206\u6790\u73AF\u5883\u6570\u636E\u751F\u6210\u79CD\u690D\u5EFA\u8BAE'}
            </Text>
          </div>
          <Button
            type="primary"
            icon={<ThunderboltOutlined />}
            loading={loading}
            onClick={generateReport}
          >
            {'\u751F\u6210\u62A5\u544A'}
          </Button>
        </Flex>
      </div>

      {!aiConfigured && (
        <Alert
          type="warning"
          showIcon
          message={'\u{1F9E0} AI \u672A\u914D\u7F6E'}
          description={'\u8BF7\u5148\u524D\u5F80\u201C\u7CFB\u7EDF\u4FE1\u606F\u201D\u9875\u9762\u914D\u7F6E MiniMax API Key\uFF0C\u624D\u80FD\u4F7F\u7528\u667A\u80FD\u62A5\u544A\u529F\u80FD\u3002'}
          style={{ marginBottom: 16 }}
        />
      )}

      {/* 当前环境概要 */}
      <Card size="small" className="ind-card" style={{ marginBottom: 16, borderLeft: '3px solid #475569' }}>
        <Text strong style={{ fontSize: 13 }}>{'\u{1F4CA}'} {'\u5F53\u524D\u73AF\u5883\u6982\u89C8'}</Text>
        <Flex gap={12} wrap style={{ marginTop: 8 }}>
          <Tag>{'\u{1F321}\uFE0F'} {current.temp?.toFixed(1) ?? '--'}{'\u00B0C'}</Tag>
          <Tag>{'\u{1F4A7}'} {current.hum?.toFixed(1) ?? '--'}%</Tag>
          <Tag>{'\u2600\uFE0F'} {current.lux ?? '--'}lx</Tag>
          <Tag>{'\u{1F331}'} {current.soil ?? '--'}%</Tag>
          <Tag>{'\u2601\uFE0F'} {current.eco2 ?? '--'}ppm</Tag>
          <Tag>{'\u{1F9EA}'} {current.tvoc ?? '--'}ppb</Tag>
        </Flex>
      </Card>

      {loading && (
        <Card className="ind-card" style={{ marginBottom: 16, textAlign: 'center', padding: 40 }}>
          <Spin size="large" />
          <div style={{ marginTop: 16 }}>
            <Text type="secondary">{'\u{1F9E0}'} AI {'\u6B63\u5728\u5206\u6790\u73AF\u5883\u6570\u636E\uFF0C\u8BF7\u7A0D\u5019...'}</Text>
          </div>
        </Card>
      )}

      {report && !loading && (
        <Card
          className="ind-card"
          style={{ marginBottom: 16, borderLeft: `3px solid ${scoreColor(report.score)}` }}
          title={
            <Flex justify="space-between" align="center">
              <Text strong><FileTextOutlined /> {'\u73AF\u5883\u5206\u6790\u62A5\u544A'}</Text>
              <Text type="secondary" style={{ fontSize: 11 }}>{report.timestamp}</Text>
            </Flex>
          }
        >
          {/* 健康评分 */}
          <Flex align="center" gap={16} style={{ marginBottom: 16 }}>
            <Progress
              type="dashboard"
              percent={report.score}
              size={80}
              strokeColor={scoreColor(report.score)}
              format={(pct) => <span style={{ fontSize: 18, fontWeight: 700 }}>{pct}</span>}
            />
            <div>
              <Text strong style={{ fontSize: 18, color: scoreColor(report.score) }}>
                {scoreLabel(report.score)}
              </Text>
              <br />
              <Text type="secondary" style={{ fontSize: 12 }}>{'\u73AF\u5883\u5065\u5EB7\u8BC4\u5206'}</Text>
            </div>
          </Flex>

          {/* 分析摘要 */}
          <Card size="small" style={{ marginBottom: 12, background: '#f8fafc' }}>
            <Text strong style={{ fontSize: 12, color: '#475569' }}>{'\u{1F4CB}'} {'\u5206\u6790\u6458\u8981'}</Text>
            <Paragraph style={{ margin: '8px 0 0', fontSize: 13, whiteSpace: 'pre-wrap' }}>
              {report.summary}
            </Paragraph>
          </Card>

          {/* 优化建议 */}
          {report.suggestions.length > 0 && (
            <Card size="small" style={{ background: '#f0fdf4' }}>
              <Text strong style={{ fontSize: 12, color: '#166534' }}>{'\u{1F4A1}'} {'\u4F18\u5316\u5EFA\u8BAE'}</Text>
              <ul style={{ margin: '8px 0 0', paddingLeft: 20 }}>
                {report.suggestions.map((s, i) => (
                  <li key={i} style={{ fontSize: 13, lineHeight: 1.8 }}>{s}</li>
                ))}
              </ul>
            </Card>
          )}
        </Card>
      )}

      {!report && !loading && (
        <Card className="ind-card">
          <Empty description={'\u70B9\u51FB\u201C\u751F\u6210\u62A5\u544A\u201D\u83B7\u53D6 AI \u667A\u80FD\u5206\u6790'} />
        </Card>
      )}

      {/* 历史报告 */}
      {history.length > 1 && (
        <Card
          size="small"
          className="ind-card"
          title={<><ReloadOutlined /> {'\u5386\u53F2\u62A5\u544A'}</>}
          style={{ marginTop: 16 }}
        >
          {history.slice(1).map((r, i) => (
            <Card
              key={i}
              size="small"
              style={{ marginBottom: 8, cursor: 'pointer', borderLeft: `3px solid ${scoreColor(r.score)}` }}
              onClick={() => setReport(r)}
            >
              <Flex justify="space-between" align="center">
                <Flex align="center" gap={8}>
                  <Tag color={scoreColor(r.score)} style={{ fontWeight: 700 }}>{r.score}{'\u5206'}</Tag>
                  <Text style={{ fontSize: 12 }}>{scoreLabel(r.score)}</Text>
                </Flex>
                <Text type="secondary" style={{ fontSize: 11 }}>{r.timestamp}</Text>
              </Flex>
            </Card>
          ))}
        </Card>
      )}
    </>
  );
}
