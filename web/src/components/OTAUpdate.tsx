import { useState, useRef } from 'react';
import { Card, Button, Progress, Typography, Flex, Alert, Steps, Tag, App } from 'antd';
import { UploadOutlined, CloudUploadOutlined, CheckCircleOutlined, WarningOutlined } from '@ant-design/icons';

const { Title, Text } = Typography;

type OtaStatus = 'idle' | 'selected' | 'uploading' | 'success' | 'error';

export default function OTAUpdate() {
  const { message: msg, modal } = App.useApp();
  const [status, setStatus] = useState<OtaStatus>('idle');
  const [file, setFile] = useState<File | null>(null);
  const [progress, setProgress] = useState(0);
  const [errorMsg, setErrorMsg] = useState('');
  const fileRef = useRef<HTMLInputElement>(null);

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    const f = e.target.files?.[0];
    if (!f) return;
    if (!f.name.endsWith('.bin')) {
      msg.error('\u8BF7\u9009\u62E9 .bin \u56FA\u4EF6\u6587\u4EF6');
      return;
    }
    if (f.size > 2 * 1024 * 1024) {
      msg.error('\u56FA\u4EF6\u6587\u4EF6\u4E0D\u80FD\u8D85\u8FC7 2MB');
      return;
    }
    setFile(f);
    setStatus('selected');
    setErrorMsg('');
  };

  const startUpload = () => {
    modal.confirm({
      title: '\u2697\uFE0F \u786E\u8BA4\u56FA\u4EF6\u5347\u7EA7',
      content: `\u5373\u5C06\u4E0A\u4F20 ${file!.name} (${(file!.size / 1024).toFixed(1)} KB)\u3002\u5347\u7EA7\u671F\u95F4\u8BBE\u5907\u5C06\u91CD\u542F\uFF0C\u8BF7\u52FF\u65AD\u7535\u3002`,
      okText: '\u5F00\u59CB\u5347\u7EA7',
      cancelText: '\u53D6\u6D88',
      okButtonProps: { danger: true },
      onOk: doUpload,
    });
  };

  const doUpload = async () => {
    if (!file) return;
    setStatus('uploading');
    setProgress(0);

    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/ota', true);

    xhr.upload.onprogress = (e) => {
      if (e.lengthComputable) setProgress(Math.round((e.loaded / e.total) * 100));
    };

    xhr.onload = () => {
      if (xhr.status === 200) {
        setStatus('success');
        setProgress(100);
        msg.success('\u56FA\u4EF6\u5347\u7EA7\u6210\u529F\uFF0C\u8BBE\u5907\u6B63\u5728\u91CD\u542F...');
      } else {
        setStatus('error');
        setErrorMsg(xhr.responseText || `HTTP ${xhr.status}`);
      }
    };

    xhr.onerror = () => {
      setStatus('error');
      setErrorMsg('\u7F51\u7EDC\u8FDE\u63A5\u5931\u8D25');
    };

    const formData = new FormData();
    formData.append('firmware', file);
    xhr.send(formData);
  };

  const reset = () => {
    setStatus('idle');
    setFile(null);
    setProgress(0);
    setErrorMsg('');
    if (fileRef.current) fileRef.current.value = '';
  };

  const stepIndex = status === 'idle' ? 0 : status === 'selected' ? 1 : status === 'uploading' ? 2 : 3;

  return (
    <>
      <div className="page-header">
        <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>
          {'\u2699\uFE0F'} OTA {'\u56FA\u4EF6\u5347\u7EA7'}
        </Title>
      </div>

      <Card className="ind-card" style={{ marginBottom: 16 }}>
        <Steps
          current={stepIndex}
          size="small"
          items={[
            { title: '\u9009\u62E9\u56FA\u4EF6' },
            { title: '\u786E\u8BA4\u4FE1\u606F' },
            { title: '\u4E0A\u4F20\u5347\u7EA7' },
            { title: status === 'success' ? '\u5B8C\u6210' : status === 'error' ? '\u5931\u8D25' : '\u7ED3\u679C' },
          ]}
        />
      </Card>

      <Alert
        type="warning"
        showIcon
        icon={<WarningOutlined />}
        message={'\u6CE8\u610F\u4E8B\u9879'}
        description={
          <ul style={{ margin: '4px 0 0', paddingLeft: 20, fontSize: 12 }}>
            <li>{'\u4EC5\u652F\u6301 .bin \u683C\u5F0F\u7684 ESP32-S3 \u56FA\u4EF6\u6587\u4EF6'}</li>
            <li>{'\u5347\u7EA7\u671F\u95F4\u8BF7\u52FF\u65AD\u7535\u6216\u5173\u95ED\u6D4F\u89C8\u5668'}</li>
            <li>{'\u5347\u7EA7\u5B8C\u6210\u540E\u8BBE\u5907\u5C06\u81EA\u52A8\u91CD\u542F\uFF0C\u9700\u91CD\u65B0\u8FDE\u63A5'}</li>
            <li>{'\u8BF7\u786E\u4FDD\u56FA\u4EF6\u7248\u672C\u6B63\u786E\uFF0C\u9519\u8BEF\u7684\u56FA\u4EF6\u53EF\u80FD\u5BFC\u81F4\u8BBE\u5907\u53D8\u7816'}</li>
          </ul>
        }
        style={{ marginBottom: 16 }}
      />

      <Card className="ind-card">
        {status === 'idle' && (
          <Flex vertical align="center" gap={16} style={{ padding: 24 }}>
            <CloudUploadOutlined style={{ fontSize: 48, color: '#94a3b8' }} />
            <Text type="secondary">{'\u9009\u62E9\u56FA\u4EF6\u6587\u4EF6\u5F00\u59CB\u5347\u7EA7'}</Text>
            <input
              ref={fileRef}
              type="file"
              accept=".bin"
              onChange={handleFileSelect}
              style={{ display: 'none' }}
            />
            <Button type="primary" icon={<UploadOutlined />} onClick={() => fileRef.current?.click()}>
              {'\u9009\u62E9\u56FA\u4EF6 (.bin)'}
            </Button>
          </Flex>
        )}

        {status === 'selected' && file && (
          <Flex vertical gap={16} style={{ padding: 16 }}>
            <Card size="small" style={{ background: '#f8fafc' }}>
              <Flex justify="space-between" align="center">
                <div>
                  <Text strong>{'\u{1F4C4}'} {file.name}</Text>
                  <br />
                  <Text type="secondary" style={{ fontSize: 12 }}>
                    {'\u5927\u5C0F'}: {(file.size / 1024).toFixed(1)} KB
                  </Text>
                </div>
                <Tag color="blue">{'\u5DF2\u9009\u62E9'}</Tag>
              </Flex>
            </Card>
            <Flex gap={8}>
              <Button type="primary" danger icon={<UploadOutlined />} onClick={startUpload}>
                {'\u5F00\u59CB\u5347\u7EA7'}
              </Button>
              <Button onClick={reset}>{'\u91CD\u65B0\u9009\u62E9'}</Button>
            </Flex>
          </Flex>
        )}

        {status === 'uploading' && (
          <Flex vertical align="center" gap={16} style={{ padding: 24 }}>
            <Progress type="circle" percent={progress} size={120} strokeColor="#475569" />
            <Text strong>{'\u6B63\u5728\u4E0A\u4F20\u56FA\u4EF6...'}</Text>
            <Text type="secondary" style={{ fontSize: 12 }}>{'\u8BF7\u52FF\u5173\u95ED\u9875\u9762\u6216\u65AD\u7535'}</Text>
          </Flex>
        )}

        {status === 'success' && (
          <Flex vertical align="center" gap={16} style={{ padding: 24 }}>
            <CheckCircleOutlined style={{ fontSize: 48, color: '#22c55e' }} />
            <Text strong style={{ color: '#22c55e', fontSize: 16 }}>{'\u{2705}'} {'\u5347\u7EA7\u6210\u529F\uFF01'}</Text>
            <Text type="secondary">{'\u8BBE\u5907\u6B63\u5728\u91CD\u542F\uFF0C\u8BF7\u7A0D\u5019\u540E\u5237\u65B0\u9875\u9762'}</Text>
            <Button onClick={() => window.location.reload()}>{'\u5237\u65B0\u9875\u9762'}</Button>
          </Flex>
        )}

        {status === 'error' && (
          <Flex vertical align="center" gap={16} style={{ padding: 24 }}>
            <WarningOutlined style={{ fontSize: 48, color: '#ef4444' }} />
            <Text strong style={{ color: '#ef4444', fontSize: 16 }}>{'\u274C'} {'\u5347\u7EA7\u5931\u8D25'}</Text>
            <Text type="secondary">{errorMsg}</Text>
            <Button type="primary" onClick={reset}>{'\u91CD\u8BD5'}</Button>
          </Flex>
        )}
      </Card>
    </>
  );
}
