import * as echarts from 'echarts/core';
import { LineChart } from 'echarts/charts';
import {
  TitleComponent,
  TooltipComponent,
  LegendComponent,
  GridComponent,
} from 'echarts/components';
import { CanvasRenderer } from 'echarts/renderers';
import type { EChartsOption } from 'echarts';

echarts.use([LineChart, TitleComponent, TooltipComponent, LegendComponent, GridComponent, CanvasRenderer]);

export { echarts };
export type { EChartsOption };

export function makeChartOpt(
  title: string,
  names: [string, string],
  colors: [string, string],
  yNames: [string, string],
  yMin?: number,
): EChartsOption {
  return {
    title: { text: title, left: 'center', textStyle: { color: '#475569', fontSize: 13 } },
    tooltip: { trigger: 'axis' },
    legend: { data: names, top: 22, itemWidth: 10, itemHeight: 10, textStyle: { fontSize: 11 } },
    grid: { left: '14%', right: '14%', bottom: '10%', top: '28%' },
    xAxis: { type: 'category', data: [] },
    yAxis: [
      { type: 'value', name: yNames[0], min: yMin },
      { type: 'value', name: yNames[1], position: 'right', min: yNames[1] === '%' ? 0 : undefined, max: yNames[1] === '%' ? 100 : undefined },
    ],
    series: [
      { name: names[0], type: 'line', data: [], smooth: true, itemStyle: { color: colors[0] }, areaStyle: { opacity: 0.08 }, symbol: 'none' },
      { name: names[1], type: 'line', yAxisIndex: 1, data: [], smooth: true, itemStyle: { color: colors[1] }, areaStyle: { opacity: 0.08 }, symbol: 'none' },
    ],
  };
}

export function calcStats(arr: number[]) {
  if (!arr || arr.length === 0) return null;
  let min = Infinity, max = -Infinity, sum = 0;
  for (const v of arr) { if (v < min) min = v; if (v > max) max = v; sum += v; }
  return { min, max, avg: sum / arr.length };
}
