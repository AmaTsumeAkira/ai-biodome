import { useRef, useMemo, useState, useCallback, useEffect } from 'react';
import { Canvas, useFrame, ThreeEvent } from '@react-three/fiber';
import { OrbitControls, Html, RoundedBox, Grid, Line } from '@react-three/drei';
import { EffectComposer, Bloom, N8AO, Vignette } from '@react-three/postprocessing';
import * as THREE from 'three';
import { Card, Typography, Flex, Tag } from 'antd';
import { useApp } from '../context/AppContext';

const { Title, Text: AText } = Typography;

/* ========== 工具函数 ========== */
function lerpColor(val: number, min: number, max: number, cLow: string, cHigh: string): string {
  const t = Math.max(0, Math.min(1, (val - min) / (max - min)));
  const p = (c: string) => [parseInt(c.slice(1, 3), 16), parseInt(c.slice(3, 5), 16), parseInt(c.slice(5, 7), 16)];
  const [r1, g1, b1] = p(cLow);
  const [r2, g2, b2] = p(cHigh);
  return `rgb(${Math.round(r1 + (r2 - r1) * t)},${Math.round(g1 + (g2 - g1) * t)},${Math.round(b1 + (b2 - b1) * t)})`;
}

function seededRand(seed: number) {
  const x = Math.sin(seed * 127.1 + 311.7) * 43758.5453;
  return x - Math.floor(x);
}

/* ========== 矿洞隧道结构 ========== */
function MineTunnel() {
  const tunnelGeo = useMemo(() => {
    const w = 6, len = 14, h = 3.5;
    const segments = 24;
    const positions: number[] = [];
    const indices: number[] = [];
    const normals: number[] = [];
    const uvs: number[] = [];

    const archPoints: [number, number][] = [];
    archPoints.push([-w / 2, 0]);
    archPoints.push([-w / 2, h]);
    for (let i = 0; i <= segments; i++) {
      const angle = Math.PI - (i / segments) * Math.PI;
      archPoints.push([Math.cos(angle) * (w / 2), h + Math.sin(angle) * (w / 2)]);
    }
    archPoints.push([w / 2, h]);
    archPoints.push([w / 2, 0]);

    const zSteps = 20;
    const zMin = -len / 2, zMax = len / 2;
    for (let zi = 0; zi <= zSteps; zi++) {
      const z = zMin + (zi / zSteps) * (zMax - zMin);
      const v = zi / zSteps;
      for (let pi = 0; pi < archPoints.length; pi++) {
        const [x, y] = archPoints[pi];
        const bump = (seededRand(zi * 100 + pi) - 0.5) * 0.15;
        const bx = x + (x > 0 ? bump : -bump);
        const by = y + bump * 0.5;
        positions.push(bx, by, z);
        const cx = 0, cy = h;
        const nx = cx - x, ny = cy - y;
        const nl = Math.sqrt(nx * nx + ny * ny) || 1;
        normals.push(nx / nl, ny / nl, 0);
        uvs.push(pi / (archPoints.length - 1), v);
      }
    }
    const stride = archPoints.length;
    for (let zi = 0; zi < zSteps; zi++) {
      for (let pi = 0; pi < stride - 1; pi++) {
        const a = zi * stride + pi;
        const b = a + 1;
        const c = a + stride;
        const d = c + 1;
        indices.push(a, c, b, b, c, d);
      }
    }
    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
    geo.setAttribute('normal', new THREE.Float32BufferAttribute(normals, 3));
    geo.setAttribute('uv', new THREE.Float32BufferAttribute(uvs, 2));
    geo.setIndex(indices);
    geo.computeVertexNormals();
    return geo;
  }, []);

  return (
    <group>
      {/* 岩石隧道�?*/}
      <mesh geometry={tunnelGeo}>
        <meshStandardMaterial color="#6b5a48" roughness={0.95} metalness={0.05} side={THREE.BackSide} />
      </mesh>

      {/* 地面 */}
      <mesh rotation-x={-Math.PI / 2} position={[0, 0.01, 0]} receiveShadow>
        <planeGeometry args={[6, 14]} />
        <meshStandardMaterial color="#5a4a38" roughness={0.9} />
      </mesh>

      {/* 中央走道 */}
      <mesh rotation-x={-Math.PI / 2} position={[0, 0.02, 0]}>
        <planeGeometry args={[1.6, 14]} />
        <meshStandardMaterial color="#6e6050" roughness={0.85} />
      </mesh>

      {/* 矿道铁轨 */}
      {[-0.6, 0.6].map((x) => (
        <mesh key={x} position={[x, 0.04, 0]}>
          <boxGeometry args={[0.06, 0.04, 14]} />
          <meshStandardMaterial color="#8a7d6f" metalness={0.7} roughness={0.3} />
        </mesh>
      ))}
      {/* 枕木 */}
      {Array.from({ length: 24 }, (_, i) => (
        <mesh key={i} position={[0, 0.02, -6.8 + i * 0.58]}>
          <boxGeometry args={[1.6, 0.04, 0.1]} />
          <meshStandardMaterial color="#6b5240" roughness={0.85} />
        </mesh>
      ))}

      {/* 木质支撑�?*/}
      {[-5, -2.5, 0, 2.5, 5].map((z) => (
        <group key={z}>
          <mesh position={[-2.8, 1.75, z]}>
            <boxGeometry args={[0.22, 3.5, 0.22]} />
            <meshStandardMaterial color="#8b6914" roughness={0.8} />
          </mesh>
          <mesh position={[2.8, 1.75, z]}>
            <boxGeometry args={[0.22, 3.5, 0.22]} />
            <meshStandardMaterial color="#8b6914" roughness={0.8} />
          </mesh>
          <mesh position={[0, 3.6, z]}>
            <boxGeometry args={[5.8, 0.22, 0.22]} />
            <meshStandardMaterial color="#8b6914" roughness={0.8} />
          </mesh>
          <mesh position={[-2.3, 3.3, z]} rotation-z={0.4}>
            <boxGeometry args={[0.14, 1.2, 0.14]} />
            <meshStandardMaterial color="#7a5c12" roughness={0.85} />
          </mesh>
          <mesh position={[2.3, 3.3, z]} rotation-z={-0.4}>
            <boxGeometry args={[0.14, 1.2, 0.14]} />
            <meshStandardMaterial color="#7a5c12" roughness={0.85} />
          </mesh>
          {[[-2.8, 3.4, z], [2.8, 3.4, z]].map((p, bi) => (
            <mesh key={bi} position={[p[0] + (bi === 0 ? 0.12 : -0.12), p[1], p[2]]}>
              <cylinderGeometry args={[0.025, 0.025, 0.05, 6]} />
              <meshStandardMaterial color="#555" metalness={0.8} />
            </mesh>
          ))}
        </group>
      ))}

      {/* 岩壁石块装饰 */}
      {[
        [-2.9, 2.5, -3.5, 0.2], [-2.95, 1.2, 1, 0.15], [2.9, 2.0, -1.5, 0.18],
        [2.95, 1.5, 3.5, 0.22], [-2.85, 0.5, 4, 0.16], [2.88, 3.0, -4, 0.14],
        [-2.92, 3.2, 2.5, 0.12], [2.92, 0.8, -5, 0.2],
        [-2.88, 1.8, -5.5, 0.17], [2.91, 2.3, 5.5, 0.13], [-2.93, 0.3, -2, 0.19],
        [2.87, 1.0, 1.5, 0.15], [-2.86, 2.8, 0.5, 0.11], [2.93, 3.3, -2.5, 0.16],
      ].map(([x, y, z, size], i) => (
        <mesh key={i} position={[x, y, z]}>
          <dodecahedronGeometry args={[size, 0]} />
          <meshStandardMaterial
            color={['#6b5b4f', '#7a6a5e', '#8a7a6e', '#5d4d3f'][i % 4]}
            roughness={0.95}
          />
        </mesh>
      ))}

      {/* 钟乳�?*/}
      {[
        [0.8, 5.8, -3, 0.4], [-0.5, 6.0, 1.5, 0.35], [1.5, 5.5, -5, 0.3],
        [-1.2, 5.7, 4, 0.38], [0.3, 6.1, -1, 0.25], [-0.8, 5.9, 5.5, 0.32],
        [2.0, 5.3, 3, 0.28], [-1.8, 5.6, -4.5, 0.36],
      ].map(([x, y, z, h], i) => (
        <mesh key={`stal${i}`} position={[x, y, z]}>
          <coneGeometry args={[0.06 + seededRand(i) * 0.04, h, 5]} />
          <meshStandardMaterial color="#8a8070" roughness={0.9} />
        </mesh>
      ))}

      {/* 通风管道 */}
      <mesh position={[2.2, 5.2, 0]} rotation-x={Math.PI / 2}>
        <cylinderGeometry args={[0.28, 0.28, 13, 10]} />
        <meshStandardMaterial color="#708090" metalness={0.5} roughness={0.4} />
      </mesh>
      {[-5, -2, 1, 4].map((z) => (
        <group key={z}>
          <mesh position={[2.2, 5.2, z]} rotation-x={Math.PI / 2}>
            <torusGeometry args={[0.3, 0.03, 6, 12]} />
            <meshStandardMaterial color="#607080" metalness={0.6} />
          </mesh>
          <mesh position={[2.2, 4.85, z]}>
            <boxGeometry args={[0.06, 0.5, 0.06]} />
            <meshStandardMaterial color="#607080" metalness={0.4} />
          </mesh>
        </group>
      ))}

      {/* 电缆线束 */}
      {[2.8, 2.65, 2.5].map((y, i) => (
        <mesh key={i} position={[-2.72, y, 0]} rotation-x={Math.PI / 2}>
          <cylinderGeometry args={[0.025, 0.025, 13, 5]} />
          <meshStandardMaterial color={['#1a1a1a', '#cc2200', '#1565c0'][i]} />
        </mesh>
      ))}
      {[-5, -2.5, 0, 2.5, 5].map((z) => (
        <mesh key={z} position={[-2.72, 2.65, z]}>
          <boxGeometry args={[0.04, 0.2, 0.04]} />
          <meshStandardMaterial color="#555" metalness={0.5} />
        </mesh>
      ))}

      {/* 水洼 */}
      {[
        [0, 0.025, -4.5, 0.5], [-1, 0.025, 2, 0.4],
        [1.2, 0.025, 5, 0.35], [0.5, 0.025, -1, 0.3],
      ].map(([x, y, z, r], i) => (
        <mesh key={`puddle${i}`} position={[x, y, z]} rotation-x={-Math.PI / 2}>
          <circleGeometry args={[r, 12]} />
          <meshStandardMaterial
            color="#4a6070"
            metalness={0.6}
            roughness={0.1}
            transparent
            opacity={0.5}
          />
        </mesh>
      ))}

      {/* 矿车占位 - �?MineCart 组件渲染 */}

      {/* 安全标志�?*/}
      <group position={[2.72, 2.8, -3]}>
        <mesh>
          <boxGeometry args={[0.02, 0.35, 0.5]} />
          <meshStandardMaterial color="#ffeb3b" />
        </mesh>
        <mesh position={[0.012, 0.02, 0]}>
          <boxGeometry args={[0.005, 0.15, 0.15]} />
          <meshStandardMaterial color="#212121" />
        </mesh>
      </group>

      {/* 传感器硬件盒�?(RoundedBox) */}
      {[
        [-2.65, 2.0, -2, '#37474f'],
        [-2.65, 1.5, 3, '#455a64'],
        [2.65, 1.8, 0, '#37474f'],
      ].map(([x, y, z, color], i) => (
        <group key={`hw${i}`} position={[x as number, y as number, z as number]}>
          <RoundedBox args={[0.1, 0.15, 0.2]} radius={0.015} smoothness={4}>
            <meshStandardMaterial color={color as string} metalness={0.3} roughness={0.5} />
          </RoundedBox>
          <mesh position={[(x as number) > 0 ? -0.06 : 0.06, 0.05, 0]}>
            <sphereGeometry args={[0.015, 4, 4]} />
            <meshStandardMaterial color="#00e676" emissive="#00e676" emissiveIntensity={2} />
          </mesh>
          <mesh position={[0, 0.12, 0]}>
            <cylinderGeometry args={[0.005, 0.005, 0.1, 3]} />
            <meshStandardMaterial color="#333" />
          </mesh>
        </group>
      ))}
    </group>
  );
}

/* ========== 浮尘粒子 ========== */
function DustParticles() {
  const ref = useRef<THREE.Points>(null);
  const count = 120;

  const [positions, velocities] = useMemo(() => {
    const pos = new Float32Array(count * 3);
    const vel = new Float32Array(count * 3);
    for (let i = 0; i < count; i++) {
      pos[i * 3] = (Math.random() - 0.5) * 5;
      pos[i * 3 + 1] = Math.random() * 5 + 0.5;
      pos[i * 3 + 2] = (Math.random() - 0.5) * 12;
      vel[i * 3] = (Math.random() - 0.5) * 0.003;
      vel[i * 3 + 1] = (Math.random() - 0.5) * 0.001;
      vel[i * 3 + 2] = (Math.random() - 0.5) * 0.002;
    }
    return [pos, vel];
  }, []);

  useFrame(() => {
    if (!ref.current) return;
    const pos = ref.current.geometry.attributes.position as THREE.BufferAttribute;
    const arr = pos.array as Float32Array;
    for (let i = 0; i < count; i++) {
      arr[i * 3] += velocities[i * 3];
      arr[i * 3 + 1] += velocities[i * 3 + 1];
      arr[i * 3 + 2] += velocities[i * 3 + 2];
      if (Math.abs(arr[i * 3]) > 2.5) velocities[i * 3] *= -1;
      if (arr[i * 3 + 1] < 0.5 || arr[i * 3 + 1] > 5.5) velocities[i * 3 + 1] *= -1;
      if (Math.abs(arr[i * 3 + 2]) > 6) velocities[i * 3 + 2] *= -1;
    }
    pos.needsUpdate = true;
  });

  return (
    <points ref={ref}>
      <bufferGeometry>
        <bufferAttribute attach="attributes-position" args={[positions, 3]} />
      </bufferGeometry>
      <pointsMaterial size={0.04} color="#d4c8a8" transparent opacity={0.4} sizeAttenuation />
    </points>
  );
}

/* ========== 种植�?(RoundedBox) ========== */
function GrowBed({ position, count, variant }: { position: [number, number, number]; count: number; variant: number }) {
  const plantsRef = useRef<THREE.Group>(null);
  useFrame(({ clock }) => {
    if (plantsRef.current) {
      plantsRef.current.children.forEach((child, i) => {
        const t = clock.elapsedTime * 0.8 + i * 0.7 + variant;
        child.rotation.x = Math.sin(t) * 0.05;
        child.rotation.z = Math.cos(t * 0.8) * 0.05;
      });
    }
  });

  const bedColor = variant % 2 === 0 ? '#5a4a3a' : '#4f4030';
  const plantColors = [
    ['#2e7d32', '#43a047', '#66bb6a'],
    ['#1b5e20', '#388e3c', '#4caf50'],
    ['#33691e', '#558b2f', '#7cb342'],
  ];
  const colors = plantColors[variant % 3];

  return (
    <group position={position}>
      {/* 种植�?(圆角) */}
      <RoundedBox args={[1.6, 0.3, 0.6]} radius={0.03} smoothness={4} position={[0, 0.15, 0]}>
        <meshStandardMaterial color={bedColor} roughness={0.8} />
      </RoundedBox>
      {/* 槽边�?*/}
      <mesh position={[0, 0.3, 0]}>
        <boxGeometry args={[1.64, 0.02, 0.64]} />
        <meshStandardMaterial color="#4a3a2a" roughness={0.9} />
      </mesh>
      {/* 土壤 */}
      <mesh position={[0, 0.31, 0]}>
        <boxGeometry args={[1.5, 0.02, 0.5]} />
        <meshStandardMaterial color="#3d2b1a" roughness={1} />
      </mesh>
      {/* 金属架腿 */}
      {[[-0.72, 0, -0.27], [0.72, 0, -0.27], [-0.72, 0, 0.27], [0.72, 0, 0.27]].map((p, i) => (
        <mesh key={i} position={p as [number, number, number]}>
          <cylinderGeometry args={[0.025, 0.03, 0.3, 6]} />
          <meshStandardMaterial color="#708090" metalness={0.6} />
        </mesh>
      ))}
      <mesh position={[0, -0.05, 0]}>
        <boxGeometry args={[1.5, 0.02, 0.02]} />
        <meshStandardMaterial color="#708090" metalness={0.5} />
      </mesh>

      {/* 植物 (风吹摇摆) */}
      <group ref={plantsRef}>
        {Array.from({ length: count }, (_, i) => {
          const x = -0.55 + (i / (count - 1)) * 1.1;
          const plantH = 0.2 + seededRand(variant * 10 + i) * 0.15;
          return (
            <group key={i} position={[x, 0.4, 0]}>
              <mesh position={[0, plantH / 2, 0]}>
                <cylinderGeometry args={[0.008, 0.015, plantH, 4]} />
                <meshStandardMaterial color={colors[0]} />
              </mesh>
              {[0, 90, 180, 270].map((angle, j) => {
                const leafY = plantH * 0.4 + j * plantH * 0.15;
                const leafSize = 0.06 + seededRand(variant * 100 + i * 10 + j) * 0.04;
                return (
                  <mesh key={j}
                    position={[
                      Math.cos((angle * Math.PI) / 180) * 0.05,
                      leafY,
                      Math.sin((angle * Math.PI) / 180) * 0.05,
                    ]}
                    rotation={[0.3 + j * 0.1, (angle * Math.PI) / 180, 0.4]}
                  >
                    <sphereGeometry args={[leafSize, 5, 4]} />
                    <meshStandardMaterial color={colors[j % 3]} />
                  </mesh>
                );
              })}
              <mesh position={[0, plantH + 0.02, 0]}>
                <sphereGeometry args={[0.03, 4, 4]} />
                <meshStandardMaterial color={colors[2]} />
              </mesh>
            </group>
          );
        })}
      </group>
    </group>
  );
}

/* ========== 矿车动画 ========== */
function MineCart() {
  const ref = useRef<THREE.Group>(null);
  const wheelRefs = useRef<THREE.Mesh[]>([]);
  useFrame(({ clock }) => {
    if (!ref.current) return;
    const t = clock.elapsedTime * 0.3;
    ref.current.position.z = 5.5 + Math.sin(t) * 3;
    wheelRefs.current.forEach((w) => {
      if (w) w.rotation.x += 0.02 * Math.cos(t);
    });
  });
  return (
    <group ref={ref} position={[0, 0.15, 5.5]}>
      {/* 车斗 (圆角) */}
      <RoundedBox args={[0.9, 0.4, 0.6]} radius={0.02} smoothness={4} position={[0, 0.25, 0]}>
        <meshStandardMaterial color="#6d6050" metalness={0.3} roughness={0.7} />
      </RoundedBox>
      <mesh position={[0, 0.35, 0]}>
        <boxGeometry args={[0.78, 0.3, 0.48]} />
        <meshStandardMaterial color="#5a4d3e" roughness={0.9} side={THREE.BackSide} />
      </mesh>
      {[[-0.15, 0.45, 0.05], [0.12, 0.42, -0.08], [0, 0.48, 0]].map((p, i) => (
        <mesh key={i} position={p as [number, number, number]}>
          <dodecahedronGeometry args={[0.08, 0]} />
          <meshStandardMaterial color="#8a7a6e" roughness={0.9} />
        </mesh>
      ))}
      {[[-0.45, 0.08, 0.2], [0.45, 0.08, 0.2], [-0.45, 0.08, -0.2], [0.45, 0.08, -0.2]].map((p, i) => (
        <mesh key={i} position={p as [number, number, number]} rotation-z={Math.PI / 2}
          ref={(el: THREE.Mesh) => { if (el) wheelRefs.current[i] = el; }}>
          <cylinderGeometry args={[0.08, 0.08, 0.04, 8]} />
          <meshStandardMaterial color="#555" metalness={0.7} />
        </mesh>
      ))}
      <mesh position={[0, 0.5, -0.4]} rotation-x={-0.3}>
        <cylinderGeometry args={[0.02, 0.02, 0.5, 4]} />
        <meshStandardMaterial color="#666" metalness={0.5} />
      </mesh>
    </group>
  );
}

/* ========== 钟乳石滴�?========== */
function StalactiteDrips() {
  const dripPositions = useMemo(() => [
    [0.8, 5.6, -3], [-0.5, 5.83, 1.5], [1.5, 5.35, -5], [-1.2, 5.51, 4],
  ], []);
  const ref = useRef<THREE.Group>(null);
  useFrame(({ clock }) => {
    if (!ref.current) return;
    ref.current.children.forEach((child, i) => {
      const period = 3 + i * 0.7;
      const t = (clock.elapsedTime + i * 1.5) % period;
      const dropPhase = t / period;
      child.position.y = dripPositions[i][1] - dropPhase * 3;
      child.visible = dropPhase < 0.6;
      const mesh = child as THREE.Mesh;
      if (mesh.material instanceof THREE.MeshStandardMaterial) {
        mesh.material.opacity = Math.max(0, 0.7 - dropPhase);
      }
    });
  });
  return (
    <group ref={ref}>
      {dripPositions.map(([x, y, z], i) => (
        <mesh key={i} position={[x, y, z]}>
          <sphereGeometry args={[0.025, 5, 5]} />
          <meshStandardMaterial color="#7ab8d4" transparent opacity={0.7} />
        </mesh>
      ))}
    </group>
  );
}

/* ========== 矿用排风�?(可悬�? ========== */
function MineFan({ position, active, hovered }: { position: [number, number, number]; active: boolean; hovered: boolean }) {
  const ref = useRef<THREE.Group>(null);
  useFrame((_, delta) => {
    if (ref.current && active) ref.current.rotation.z += delta * 10;
  });
  return (
    <group position={position}>
      <mesh rotation-y={Math.PI / 2}>
        <torusGeometry args={[0.6, 0.04, 6, 16]} />
        <meshStandardMaterial color={hovered ? '#5a7a84' : '#455a64'} metalness={0.6}
          emissive={hovered ? '#ffffff' : '#000000'} emissiveIntensity={hovered ? 0.1 : 0} />
      </mesh>
      {[0, 90].map((angle) => (
        <mesh key={angle} rotation-z={(angle * Math.PI) / 180}>
          <boxGeometry args={[0.03, 1.2, 0.02]} />
          <meshStandardMaterial color="#546e7a" metalness={0.5} />
        </mesh>
      ))}
      <group ref={ref}>
        {[0, 72, 144, 216, 288].map((angle) => (
          <mesh key={angle} rotation-z={(angle * Math.PI) / 180}>
            <boxGeometry args={[0.1, 0.48, 0.015]} />
            <meshStandardMaterial color={active ? '#00acc1' : '#78909c'} metalness={0.3} />
          </mesh>
        ))}
      </group>
      <mesh position={[0, 0, -0.08]}>
        <cylinderGeometry args={[0.12, 0.12, 0.15, 8]} />
        <meshStandardMaterial color="#37474f" metalness={0.6} />
      </mesh>
      <mesh position={[0, -0.65, -0.05]}>
        <boxGeometry args={[0.3, 0.06, 0.15]} />
        <meshStandardMaterial color="#455a64" metalness={0.4} />
      </mesh>
      <mesh position={[0.65, 0.55, 0]}>
        <sphereGeometry args={[0.04, 6, 6]} />
        <meshStandardMaterial
          color={active ? '#00e676' : '#616161'}
          emissive={active ? '#00e676' : '#000'}
          emissiveIntensity={active ? 2 : 0}
        />
      </mesh>
    </group>
  );
}

/* ========== LED 补光灯条 ========== */
function GrowLightStrip({ position, active }: { position: [number, number, number]; active: boolean }) {
  const ref = useRef<THREE.PointLight>(null);
  useFrame(({ clock }) => {
    if (ref.current && active) {
      ref.current.intensity = 4 + Math.sin(clock.elapsedTime * 1.5) * 0.8;
    }
  });
  return (
    <group position={position}>
      <mesh>
        <boxGeometry args={[1.8, 0.08, 0.18]} />
        <meshStandardMaterial color="#37474f" metalness={0.5} roughness={0.3} />
      </mesh>
      {Array.from({ length: 6 }, (_, i) => (
        <mesh key={`fin${i}`} position={[-0.6 + i * 0.24, 0.06, 0]}>
          <boxGeometry args={[0.02, 0.04, 0.16]} />
          <meshStandardMaterial color="#455a64" metalness={0.4} />
        </mesh>
      ))}
      {Array.from({ length: 10 }, (_, i) => (
        <mesh key={i} position={[-0.76 + i * 0.17, -0.05, 0]}>
          <boxGeometry args={[0.1, 0.015, 0.08]} />
          <meshStandardMaterial
            color={active ? '#e040fb' : '#424242'}
            emissive={active ? '#e040fb' : '#000'}
            emissiveIntensity={active ? 2 : 0}
          />
        </mesh>
      ))}
      {[-0.6, 0.6].map((x) => (
        <mesh key={x} position={[x, 0.35, 0]}>
          <cylinderGeometry args={[0.008, 0.008, 0.7, 3]} />
          <meshStandardMaterial color="#666" metalness={0.3} />
        </mesh>
      ))}
      {active && (
        <>
          <pointLight ref={ref} position={[0, -0.25, 0]} color="#e040fb" intensity={4} distance={3.5} decay={2} />
          <mesh position={[0, -0.8, 0]}>
            <cylinderGeometry args={[0.3, 0.9, 1.2, 8, 1, true]} />
            <meshStandardMaterial color="#e040fb" transparent opacity={0.03} side={THREE.DoubleSide} />
          </mesh>
        </>
      )}
    </group>
  );
}

/* ========== 加热�?(可悬�? ========== */
function MineHeater({ position, active, hovered }: { position: [number, number, number]; active: boolean; hovered: boolean }) {
  const glowRef = useRef<THREE.Mesh>(null);
  useFrame(({ clock }) => {
    if (glowRef.current && active) {
      const s = 1 + Math.sin(clock.elapsedTime * 3) * 0.2;
      glowRef.current.scale.set(s, s, s);
    }
  });
  return (
    <group position={position}>
      <RoundedBox args={[0.65, 0.85, 0.22]} radius={0.02} smoothness={4}>
        <meshStandardMaterial color={hovered ? '#4f6975' : '#37474f'} metalness={0.5} roughness={0.4}
          emissive={hovered ? '#ffffff' : '#000000'} emissiveIntensity={hovered ? 0.1 : 0} />
      </RoundedBox>
      {Array.from({ length: 5 }, (_, i) => (
        <mesh key={i} position={[0, -0.3 + i * 0.15, 0.112]}>
          <boxGeometry args={[0.5, 0.02, 0.005]} />
          <meshStandardMaterial color="#263238" metalness={0.4} />
        </mesh>
      ))}
      {[-0.2, 0, 0.2].map((y) => (
        <mesh key={y} position={[0, y, 0.12]} rotation-z={Math.PI / 2}>
          <cylinderGeometry args={[0.025, 0.025, 0.5, 6]} />
          <meshStandardMaterial
            color={active ? '#ff5722' : '#616161'}
            emissive={active ? '#ff3d00' : '#000'}
            emissiveIntensity={active ? 2 : 0}
          />
        </mesh>
      ))}
      <mesh position={[0, 0.35, 0.115]}>
        <boxGeometry args={[0.2, 0.08, 0.01]} />
        <meshStandardMaterial color="#1a237e" emissive="#1a237e" emissiveIntensity={0.3} />
      </mesh>
      {active && (
        <>
          <mesh ref={glowRef} position={[0, 0, 0.35]}>
            <sphereGeometry args={[0.5, 10, 10]} />
            <meshStandardMaterial color="#ff5722" transparent opacity={0.08} />
          </mesh>
          <pointLight position={[0, 0, 0.35]} color="#ff5722" intensity={2} distance={3.5} decay={2} />
        </>
      )}
      <mesh position={[0, 0, -0.12]}>
        <boxGeometry args={[0.3, 0.06, 0.05]} />
        <meshStandardMaterial color="#455a64" metalness={0.5} />
      </mesh>
    </group>
  );
}

/* ========== 水泵与滴�?(可悬�? ========== */
function IrrigationSystem({ position, active, hovered }: { position: [number, number, number]; active: boolean; hovered: boolean }) {
  const dripsRef = useRef<THREE.Group>(null);
  useFrame(({ clock }) => {
    if (dripsRef.current && active) {
      dripsRef.current.children.forEach((child, i) => {
        const t = (clock.elapsedTime * 2.5 + i * 0.6) % 1.5;
        child.position.y = -t * 0.6;
        const mesh = child as THREE.Mesh;
        if (mesh.material instanceof THREE.MeshStandardMaterial) {
          mesh.material.opacity = Math.max(0, 1 - t / 1.2);
        }
      });
    }
  });
  return (
    <group position={position}>
      <mesh rotation-x={Math.PI / 2}>
        <cylinderGeometry args={[0.05, 0.05, 10, 8]} />
        <meshStandardMaterial color="#1565c0" metalness={0.3} roughness={0.5} />
      </mesh>
      {[-4, -1, 2, 5].map((z) => (
        <mesh key={z} position={[0, 0, z]} rotation-x={Math.PI / 2}>
          <torusGeometry args={[0.06, 0.015, 4, 8]} />
          <meshStandardMaterial color="#0d47a1" metalness={0.4} />
        </mesh>
      ))}
      <group position={[0, 0, -5.2]}>
        <RoundedBox args={[0.4, 0.35, 0.35]} radius={0.02} smoothness={4}>
          <meshStandardMaterial color={active ? (hovered ? '#1565c0' : '#0d47a1') : (hovered ? '#6a8a9a' : '#546e7a')} metalness={0.4}
            emissive={hovered ? '#ffffff' : '#000000'} emissiveIntensity={hovered ? 0.1 : 0} />
        </RoundedBox>
        <mesh position={[0.25, 0, 0]} rotation-z={Math.PI / 2}>
          <cylinderGeometry args={[0.1, 0.1, 0.15, 8]} />
          <meshStandardMaterial color="#37474f" metalness={0.5} />
        </mesh>
        <mesh position={[0, 0.18, 0.18]}>
          <sphereGeometry args={[0.02, 4, 4]} />
          <meshStandardMaterial
            color={active ? '#2196f3' : '#666'}
            emissive={active ? '#2196f3' : '#000'}
            emissiveIntensity={active ? 2 : 0}
          />
        </mesh>
      </group>
      {[-3.5, -1.5, 0.5, 2.5, 4.5].map((z) => (
        <group key={z}>
          <mesh position={[0, -0.2, z]}>
            <cylinderGeometry args={[0.012, 0.012, 0.4, 4]} />
            <meshStandardMaterial color="#1565c0" />
          </mesh>
          <mesh position={[0, -0.4, z]}>
            <sphereGeometry args={[0.025, 4, 4]} />
            <meshStandardMaterial color="#0d47a1" />
          </mesh>
        </group>
      ))}
      {active && (
        <group ref={dripsRef}>
          {[-3.5, -1.5, 0.5, 2.5, 4.5].map((z) => (
            <mesh key={z} position={[0, -0.4, z]}>
              <sphereGeometry args={[0.03, 6, 6]} />
              <meshStandardMaterial color="#42a5f5" transparent opacity={0.8} />
            </mesh>
          ))}
        </group>
      )}
    </group>
  );
}

/* ========== 风扇气流粒子 ========== */
function WindParticles({ position, active }: { position: [number, number, number]; active: boolean }) {
  const ref = useRef<THREE.Points>(null);
  const count = 40;
  const [positions, velocities] = useMemo(() => {
    const pos = new Float32Array(count * 3);
    const vel = new Float32Array(count * 3);
    for (let i = 0; i < count; i++) {
      pos[i * 3] = (Math.random() - 0.5) * 1.2;
      pos[i * 3 + 1] = (Math.random() - 0.5) * 1.2 + 3;
      pos[i * 3 + 2] = -6.5 + Math.random() * 2;
      vel[i * 3] = (Math.random() - 0.5) * 0.01;
      vel[i * 3 + 1] = (Math.random() - 0.5) * 0.005;
      vel[i * 3 + 2] = 0.04 + Math.random() * 0.03;
    }
    return [pos, vel];
  }, []);

  useFrame(() => {
    if (!ref.current || !active) return;
    const pos = ref.current.geometry.attributes.position as THREE.BufferAttribute;
    const arr = pos.array as Float32Array;
    for (let i = 0; i < count; i++) {
      arr[i * 3] += velocities[i * 3];
      arr[i * 3 + 1] += velocities[i * 3 + 1];
      arr[i * 3 + 2] += velocities[i * 3 + 2];
      if (arr[i * 3 + 2] > position[2] + 10) {
        arr[i * 3] = position[0] + (Math.random() - 0.5) * 1.2;
        arr[i * 3 + 1] = position[1] + (Math.random() - 0.5) * 1.2;
        arr[i * 3 + 2] = position[2] + 0.5;
      }
    }
    pos.needsUpdate = true;
  });

  if (!active) return null;
  return (
    <points ref={ref}>
      <bufferGeometry>
        <bufferAttribute attach="attributes-position" args={[positions, 3]} />
      </bufferGeometry>
      <pointsMaterial size={0.03} color="#b0d8e8" transparent opacity={0.35} sizeAttenuation />
    </points>
  );
}

/* ========== 加热器热浪粒�?========== */
function HeatShimmer({ position, active }: { position: [number, number, number]; active: boolean }) {
  const ref = useRef<THREE.Points>(null);
  const count = 25;
  const [positions] = useMemo(() => {
    const pos = new Float32Array(count * 3);
    for (let i = 0; i < count; i++) {
      pos[i * 3] = position[0] + (Math.random() - 0.5) * 0.6;
      pos[i * 3 + 1] = position[1] + Math.random() * 0.5;
      pos[i * 3 + 2] = position[2] + 0.3 + Math.random() * 0.3;
    }
    return [pos];
  }, [position]);

  useFrame(({ clock }) => {
    if (!ref.current || !active) return;
    const pos = ref.current.geometry.attributes.position as THREE.BufferAttribute;
    const arr = pos.array as Float32Array;
    for (let i = 0; i < count; i++) {
      arr[i * 3 + 1] += 0.008;
      arr[i * 3] += Math.sin(clock.elapsedTime * 3 + i) * 0.002;
      if (arr[i * 3 + 1] > position[1] + 1.5) {
        arr[i * 3] = position[0] + (Math.random() - 0.5) * 0.6;
        arr[i * 3 + 1] = position[1];
        arr[i * 3 + 2] = position[2] + 0.3 + Math.random() * 0.3;
      }
    }
    pos.needsUpdate = true;
  });

  if (!active) return null;
  return (
    <points ref={ref}>
      <bufferGeometry>
        <bufferAttribute attach="attributes-position" args={[positions, 3]} />
      </bufferGeometry>
      <pointsMaterial size={0.06} color="#ff8a50" transparent opacity={0.2} sizeAttenuation />
    </points>
  );
}

/* ========== 闪烁壁灯 ========== */
function WallLight({ pos, intensity }: { pos: [number, number, number]; intensity: number }) {
  const lightRef = useRef<THREE.PointLight>(null);
  useFrame(({ clock }) => {
    if (lightRef.current) {
      const flicker = 1 + Math.sin(clock.elapsedTime * 4.7 + pos[2] * 3) * 0.08
        + Math.sin(clock.elapsedTime * 7.3 + pos[0] * 5) * 0.05;
      lightRef.current.intensity = intensity * flicker;
    }
  });
  return (
    <group>
      <pointLight ref={lightRef} position={pos} intensity={intensity} color="#ffab40" distance={8} decay={2} />
      <group position={pos}>
        <mesh rotation-x={Math.PI}>
          <coneGeometry args={[0.12, 0.08, 8, 1, true]} />
          <meshStandardMaterial color="#546e7a" metalness={0.4} side={THREE.DoubleSide} />
        </mesh>
        <mesh position={[0, -0.02, 0]}>
          <sphereGeometry args={[0.06, 8, 8]} />
          <meshStandardMaterial color="#ffcc02" emissive="#ffab40" emissiveIntensity={2.5} />
        </mesh>
        <mesh position={[pos[0] > 0 ? -0.15 : pos[0] < 0 ? 0.15 : 0, 0.05, 0]}
          rotation-z={pos[0] > 0 ? 0.5 : pos[0] < 0 ? -0.5 : 0}>
          <cylinderGeometry args={[0.01, 0.01, 0.25, 4]} />
          <meshStandardMaterial color="#555" metalness={0.5} />
        </mesh>
      </group>
    </group>
  );
}

/* ========== 传感器悬浮标�?========== */
function SensorLabel({ position, emoji, value, unit, color }: {
  position: [number, number, number]; emoji: string; value: string; unit: string; color: string;
}) {
  return (
    <Html position={position} center distanceFactor={8} style={{ pointerEvents: 'none' }}>
      <div style={{
        background: 'rgba(15,23,42,0.9)',
        border: `1.5px solid ${color}`,
        borderRadius: 6,
        padding: '4px 10px',
        fontSize: 12,
        fontWeight: 700,
        fontFamily: "'Consolas', monospace",
        color,
        whiteSpace: 'nowrap',
        boxShadow: `0 0 10px ${color}55, 0 2px 8px rgba(0,0,0,0.5)`,
        textAlign: 'center' as const,
        lineHeight: 1.3,
        backdropFilter: 'blur(4px)',
      }}>
        {emoji} {value}{unit}
      </div>
    </Html>
  );
}

/* ========== 设备状态标�?========== */
function DeviceLabel({ position, emoji, name, active }: {
  position: [number, number, number]; emoji: string; name: string; active: boolean;
}) {
  return (
    <Html position={position} center distanceFactor={8} style={{ pointerEvents: 'none' }}>
      <div style={{
        background: active ? 'rgba(0,230,118,0.12)' : 'rgba(80,80,80,0.2)',
        border: `1px solid ${active ? '#00e676' : '#555'}`,
        borderRadius: 4,
        padding: '2px 8px',
        fontSize: 10,
        fontWeight: 600,
        color: active ? '#00e676' : '#888',
        whiteSpace: 'nowrap',
        backdropFilter: 'blur(2px)',
      }}>
        {emoji} {name} <span style={{
          display: 'inline-block',
          width: 6, height: 6, borderRadius: '50%',
          background: active ? '#00e676' : '#666',
          boxShadow: active ? '0 0 6px #00e676' : 'none',
          marginLeft: 3, verticalAlign: 'middle',
        }} />
      </div>
    </Html>
  );
}

/* ========== 可点击设备信息弹�?========== */
function DeviceInfoPopup({ position, name, active, details, onClose }: {
  position: [number, number, number]; name: string; active: boolean; details: string[]; onClose: () => void;
}) {
  return (
    <Html position={position} center distanceFactor={6} style={{ pointerEvents: 'auto' }}>
      <div onClick={onClose} style={{
        background: 'rgba(15,23,42,0.95)',
        border: `1.5px solid ${active ? '#00e676' : '#f59e0b'}`,
        borderRadius: 8,
        padding: '8px 14px',
        fontSize: 11,
        color: '#e2e8f0',
        whiteSpace: 'nowrap',
        boxShadow: '0 4px 20px rgba(0,0,0,0.6)',
        cursor: 'pointer',
        backdropFilter: 'blur(6px)',
        minWidth: 100,
      }}>
        <div style={{ fontWeight: 700, fontSize: 13, marginBottom: 4, color: active ? '#00e676' : '#f59e0b' }}>
          {name} {active ? '\u2705' : '\u26A0\uFE0F'}
        </div>
        {details.map((d, i) => (
          <div key={i} style={{ color: '#94a3b8', lineHeight: 1.6 }}>{d}</div>
        ))}
        <div style={{ fontSize: 9, color: '#64748b', marginTop: 4 }}>{'\u70B9\u51FB\u5173\u95ED'}</div>
      </div>
    </Html>
  );
}

/* ========== 可悬停设备包�?========== */
function HoverableDevice({ children, onClick }: {
  children: (hovered: boolean) => React.ReactNode;
  onClick: (e: ThreeEvent<MouseEvent>) => void;
}) {
  const [hovered, setHover] = useState(false);
  useEffect(() => {
    document.body.style.cursor = hovered ? 'pointer' : 'auto';
    return () => { document.body.style.cursor = 'auto'; };
  }, [hovered]);
  return (
    <group
      onClick={onClick}
      onPointerOver={(e) => { e.stopPropagation(); setHover(true); }}
      onPointerOut={() => setHover(false)}
    >
      {children(hovered)}
    </group>
  );
}

/* ========== 3D 场景 ========== */
function Scene() {
  const { state } = useApp();
  const { current, state: devState } = state;
  const [popup, setPopup] = useState<{ name: string; pos: [number, number, number]; active: boolean; details: string[] } | null>(null);

  const tempColor = current.temp != null ? lerpColor(current.temp, 10, 40, '#3b82f6', '#ef4444') : '#94a3b8';
  const airColor = current.eco2 != null ? (current.eco2 < 600 ? '#22c55e' : current.eco2 < 1000 ? '#f59e0b' : '#ef4444') : '#94a3b8';

  const handleDeviceClick = useCallback((name: string, pos: [number, number, number], active: boolean, details: string[]) => {
    return (e: ThreeEvent<MouseEvent>) => {
      e.stopPropagation();
      setPopup((prev) => prev?.name === name ? null : { name, pos, active, details });
    };
  }, []);

  return (
    <>
      <ambientLight intensity={0.55} color="#ffeedd" />
      <directionalLight position={[0, 5, 8]} intensity={0.8} color="#fff5e0" />
      <directionalLight position={[0, 4, -6]} intensity={0.4} color="#ffe0b2" />

      {/* 闪烁壁灯 */}
      {[
        { pos: [-2.5, 2.8, -4] as [number, number, number], int: 2.5 },
        { pos: [2.5, 2.8, 2] as [number, number, number], int: 2.5 },
        { pos: [-2.5, 2.8, 5] as [number, number, number], int: 2 },
        { pos: [0, 3.5, -1] as [number, number, number], int: 2 },
        { pos: [0, 3.5, 4] as [number, number, number], int: 1.8 },
        { pos: [2.5, 2.8, -2] as [number, number, number], int: 1.5 },
      ].map(({ pos, int }, i) => (
        <WallLight key={i} pos={pos} intensity={int} />
      ))}

      <MineTunnel />
      <DustParticles />
      <StalactiteDrips />
      <MineCart />

      {/* 科技网格地面 */}
      <Grid
        position={[0, 0.015, 0]}
        args={[10.5, 20]}
        cellSize={0.5}
        cellThickness={1}
        cellColor="#6b7280"
        sectionSize={2}
        sectionThickness={1.5}
        sectionColor="#9ca3af"
        fadeDistance={15}
        fadeStrength={1}
      />

      {[-4, -1.5, 1, 3.5].map((z, i) => (
        <GrowBed key={`L${z}`} position={[-1.5, 0, z]} count={5} variant={i} />
      ))}
      {[-3, 0, 2.5, 5].map((z, i) => (
        <GrowBed key={`R${z}`} position={[1.5, 0, z]} count={5} variant={i + 4} />
      ))}

      {[-4, -1.5, 1, 3.5].map((z) => (
        <GrowLightStrip key={`LL${z}`} position={[-1.5, 2.8, z]} active={devState.light === 1} />
      ))}
      {[-3, 0, 2.5, 5].map((z) => (
        <GrowLightStrip key={`RL${z}`} position={[1.5, 2.8, z]} active={devState.light === 1} />
      ))}

      {/* 排风�?*/}
      <HoverableDevice onClick={handleDeviceClick('\u6392\u98CE\u6247', [0, 3, -6.5], devState.fan === 1, [
        `\u72B6\u6001: ${devState.fan === 1 ? '\u8FD0\u884C\u4E2D' : '\u5DF2\u505C\u6B62'}`,
        '\u529F\u7387: 25W',
        '\u98CE\u901F: 3.2m/s',
      ])}>
        {(hovered) => <MineFan position={[0, 3, -6.5]} active={devState.fan === 1} hovered={hovered} />}
      </HoverableDevice>
      <WindParticles position={[0, 3, -6.5]} active={devState.fan === 1} />

      {/* 加热�?*/}
      <HoverableDevice onClick={handleDeviceClick('\u52A0\u70ED\u5668', [-2.6, 1.2, -2], devState.heater === 1, [
        `\u72B6\u6001: ${devState.heater === 1 ? '\u52A0\u70ED\u4E2D' : '\u5DF2\u505C\u6B62'}`,
        '\u529F\u7387: 200W',
        `\u76EE\u6807: 25\u00B0C`,
      ])}>
        {(hovered) => <MineHeater position={[-2.6, 1.2, -2]} active={devState.heater === 1} hovered={hovered} />}
      </HoverableDevice>
      <HeatShimmer position={[-2.6, 1.2, -2]} active={devState.heater === 1} />

      {/* 滴灌系统 */}
      <HoverableDevice onClick={handleDeviceClick('\u6EF4\u704C\u7CFB\u7EDF', [-1.5, 2.2, 0], devState.pump === 1, [
        `\u72B6\u6001: ${devState.pump === 1 ? '\u704C\u6E89\u4E2D' : '\u5DF2\u505C\u6B62'}`,
        '\u6D41\u91CF: 2.5L/min',
        '\u6EF4\u5934: 10\u4E2A',
      ])}>
        {(hovered) => <IrrigationSystem position={[-1.5, 2.2, 0]} active={devState.pump === 1} hovered={hovered} />}
      </HoverableDevice>
      <HoverableDevice onClick={handleDeviceClick('\u6EF4\u704C\u7CFB\u7EDF\u2161', [1.5, 2.2, 0], devState.pump === 1, [
        `\u72B6\u6001: ${devState.pump === 1 ? '\u704C\u6E89\u4E2D' : '\u5DF2\u505C\u6B62'}`,
        '\u6D41\u91CF: 2.5L/min',
        '\u6EF4\u5934: 10\u4E2A',
      ])}>
        {(hovered) => <IrrigationSystem position={[1.5, 2.2, 0]} active={devState.pump === 1} hovered={hovered} />}
      </HoverableDevice>

      {/* 设备弹窗 + 牵引�?*/}
      {popup && (
        <>
          <Line
            points={[popup.pos, [popup.pos[0], popup.pos[1] + 1.2, popup.pos[2]]]}
            color={popup.active ? '#00e676' : '#f59e0b'}
            lineWidth={2}
            dashed
            dashScale={5}
          />
          <DeviceInfoPopup
            position={[popup.pos[0], popup.pos[1] + 1.2, popup.pos[2]]}
            name={popup.name}
            active={popup.active}
            details={popup.details}
            onClose={() => setPopup(null)}
          />
        </>
      )}

      <SensorLabel position={[0, 4.8, -2]} emoji={'\u{1F321}\uFE0F'}
        value={current.temp != null ? current.temp.toFixed(1) : '--'} unit={'\u00B0C'} color={tempColor} />
      <SensorLabel position={[-2.2, 1.6, 1]} emoji={'\u{1F4A7}'}
        value={current.hum != null ? current.hum.toFixed(1) : '--'} unit="%" color="#0ea5e9" />
      <SensorLabel position={[2.2, 3.2, -1]} emoji={'\u2600\uFE0F'}
        value={current.lux != null ? `${current.lux}` : '--'} unit="lx" color="#f59e0b" />
      <SensorLabel position={[0, 0.5, 2.5]} emoji={'\u{1F331}'}
        value={current.soil != null ? `${current.soil}` : '--'} unit="%" color="#22c55e" />
      <SensorLabel position={[0, 4.2, 3.5]} emoji={'\u2601\uFE0F'}
        value={current.eco2 != null ? `${current.eco2}` : '--'} unit="ppm" color={airColor} />
      <SensorLabel position={[2.2, 1.3, 4.5]} emoji={'\u{1F9EA}'}
        value={current.tvoc != null ? `${current.tvoc}` : '--'} unit="ppb" color="#8b5cf6" />

      <DeviceLabel position={[0, 3.9, -6.5]} emoji={'\u{1F32C}\uFE0F'} name={'\u6392\u98CE\u6247'} active={devState.fan === 1} />
      <DeviceLabel position={[0, 3.5, 1]} emoji={'\u{1F4A1}'} name={'\u8865\u5149\u706F'} active={devState.light === 1} />
      <DeviceLabel position={[-2.6, 2.3, -2]} emoji={'\u{1F525}'} name={'\u52A0\u70ED\u5668'} active={devState.heater === 1} />
      <DeviceLabel position={[-1.5, 2.7, -5]} emoji={'\u{1F4A7}'} name={'\u6EF4\u704C'} active={devState.pump === 1} />

      <OrbitControls
        enablePan
        enableZoom
        maxPolarAngle={Math.PI / 1.95}
        minDistance={3}
        maxDistance={16}
        target={[0, 2, 0]}
      />
    </>
  );
}

/* ========== 主组�?========== */
export default function DigitalTwin() {
  const { state } = useApp();
  const { time, state: devState } = state;
  const isAuto = devState.mode === 'auto';

  return (
    <>
      <div className="page-header">
        <Flex justify="space-between" align="center">
          <div>
            <Title level={4} style={{ margin: 0, color: '#1e293b', fontWeight: 700, letterSpacing: 0.5 }}>
              {'\u{1F3D7}\uFE0F'} {'\u6570\u5B57\u5B6A\u751F \u00B7 \u5730\u4E0B\u7EFF\u6D32'}
            </Title>
            <AText type="secondary" style={{ fontSize: 11 }} className="ind-value">{time || '--:--:--'}</AText>
          </div>
          <Tag color={isAuto ? 'success' : 'warning'} style={{ fontSize: 12, padding: '2px 12px', fontWeight: 700 }}>
            {isAuto ? '\u{1F916} \u81EA\u52A8' : '\u{1F527} \u624B\u52A8'}
          </Tag>
        </Flex>
      </div>

      <Card className="ind-card" style={{ overflow: 'hidden', boxShadow: '0 4px 20px rgba(0,0,0,0.15)', flex: 1 }}
        styles={{ body: { padding: 0, height: 'calc(100vh - 130px)', minHeight: 400, background: 'linear-gradient(180deg, #2a2010 0%, #3d2f1e 100%)' } }}
      >
        <Canvas
          camera={{ position: [5, 3.5, 8], fov: 50 }}
          shadows
          style={{ height: '100%' }}
          gl={{ antialias: true, alpha: true, toneMapping: THREE.ACESFilmicToneMapping, toneMappingExposure: 1.8 }}
        >
          <fog attach="fog" args={['#2a2010', 14, 30]} />
          <Scene />
          <EffectComposer enableNormalPass={false}>
            <N8AO halfRes color="black" aoRadius={0.5} intensity={2} />
            <Bloom luminanceThreshold={1} mipmapBlur intensity={1.5} />
            <Vignette offset={0.3} darkness={0.9} />
          </EffectComposer>
        </Canvas>
      </Card>
    </>
  );
}
