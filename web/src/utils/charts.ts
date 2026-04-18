export function calcStats(arr: number[]) {
  if (!arr || arr.length === 0) return null;
  let min = Infinity, max = -Infinity, sum = 0;
  for (const v of arr) { if (v < min) min = v; if (v > max) max = v; sum += v; }
  return { min, max, avg: sum / arr.length };
}

export interface ChartDatum {
  time: string;
  value: number;
  type: string;
}

export function toChartData(
  times: string[],
  series: { data: number[]; label: string }[],
): ChartDatum[] {
  const result: ChartDatum[] = [];
  times.forEach((t, i) => {
    for (const s of series) {
      if (s.data[i] != null) {
        result.push({ time: t, value: s.data[i], type: s.label });
      }
    }
  });
  return result;
}
