import { useRef, useEffect, useCallback } from 'react';
import { echarts } from '../utils/charts';
import type { EChartsOption } from '../utils/charts';
import type { ECharts } from 'echarts/core';

export function useChart(initialOpt: EChartsOption) {
  const containerRef = useRef<HTMLDivElement>(null);
  const chartRef = useRef<ECharts | null>(null);

  useEffect(() => {
    if (!containerRef.current) return;
    const chart = echarts.init(containerRef.current);
    chart.setOption(initialOpt);
    chartRef.current = chart;

    const onResize = () => chart.resize();
    window.addEventListener('resize', onResize);
    const ro = new ResizeObserver(() => chart.resize());
    ro.observe(containerRef.current);

    return () => {
      window.removeEventListener('resize', onResize);
      ro.disconnect();
      chart.dispose();
      chartRef.current = null;
    };
    // initialOpt is intentionally excluded — only used on mount
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const updateData = useCallback((opt: EChartsOption) => {
    chartRef.current?.setOption(opt);
  }, []);

  const resetOption = useCallback((opt: EChartsOption) => {
    chartRef.current?.setOption(opt, true);
  }, []);

  return { containerRef, updateData, resetOption, chartRef };
}
