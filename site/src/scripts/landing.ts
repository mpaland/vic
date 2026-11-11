// Landing page interactions: hero scene, reveals, magnifier, tabs.

const reduced = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

export function initLanding(): void {
	navShadow();
	scene();
	reveals();
	spotlight();
	magnifier();
	tabs();
}

function navShadow(): void {
	const nav = document.querySelector('.top');
	if (!nav) return;
	const update = () => nav.classList.toggle('scrolled', window.scrollY > 10);
	update();
	window.addEventListener('scroll', update, { passive: true });
}

/* ------------------------------------------------------------------------------------------------
 * Hero scene: an exploded 3D view of the vic rendering pipeline. The top layer (dc) shows a live
 * gauge, the layers below show how it is decomposed: primitives (gpr), native horizontal runs
 * (drv) and lit pixels (head). Crisp vector drawing at device resolution.
 */
const ACCENT = '#38bdf8';
const WHITE = '#eef0f3';
const DIM = '#2a2f37';

// display content in layer coordinates (320 x 200)
const LW = 320;
const LH = 200;
const GU = 100;
const GV = 112;
const R1 = 58;
const R2 = 72;
const A0 = 225;
const SWEEP = 270;
const DIG_X = 198;
const DIG_Y = 30;
const DIG_W = 16;
const DIG_H = 30;
const DIG_S = 24;
const BAR_X = 198;
const BAR_W = 10;
const BAR_S = 15;
const BAR_B = 180;
const BAR_H = 76;
const SEGS = [0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f]; // gfedcba

const LAYERS = [
	{ name: 'head', desc: 'pixels on the display' },
	{ name: 'drv', desc: 'native line_horz runs' },
	{ name: 'gpr', desc: 'primitives, integer math' },
	{ name: 'dc', desc: 'your drawing' },
];

function scene(): void {
	const canvas = document.getElementById('scene') as HTMLCanvasElement | null;
	if (!canvas) return;
	const ctx = canvas.getContext('2d');
	if (!ctx) return;

	const D = 1150;
	let W = 0;
	let H = 0;
	let F = 0;
	let cx = 0;
	let cy = 0;
	let labels = true;
	let cyaw = 1;
	let syaw = 0;
	let cp = 1;
	let sp = 0;
	let yOff = 0;

	const resize = () => {
		const dpr = Math.min(2, window.devicePixelRatio || 1);
		W = canvas.clientWidth;
		H = canvas.clientHeight;
		canvas.width = Math.round(W * dpr);
		canvas.height = Math.round(H * dpr);
		ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
		labels = W >= 440;
		const span = labels ? Math.min(W - 215, W * 0.66, H * 0.84) : Math.min(W * 0.8, H * 0.8);
		F = (span * D) / LW;
		cx = labels ? 20 + span * 0.62 : W * 0.5;
		cy = H * 0.5;
	};

	// layer point (u, v) at height y -> screen; returns the scale world -> px
	const pt = [0, 0, 0];
	const P = (u: number, v: number, y: number) => {
		const x = u - LW / 2;
		const z = v - LH / 2;
		const xr = x * cyaw - z * syaw;
		const zr = x * syaw + z * cyaw;
		const ry = y - yOff - D * sp;
		const rz = zr - D * cp;
		const depth = -ry * sp - rz * cp;
		const sy = ry * cp - rz * sp;
		const k = F / depth;
		pt[0] = cx + xr * k;
		pt[1] = cy - sy * k;
		pt[2] = k;
		return pt;
	};
	const mv = (u: number, v: number, y: number) => {
		P(u, v, y);
		ctx.moveTo(pt[0], pt[1]);
	};
	const ln = (u: number, v: number, y: number) => {
		P(u, v, y);
		ctx.lineTo(pt[0], pt[1]);
	};

	// rounded layer outline
	const outline: number[] = [];
	{
		const r = 12;
		const corners = [
			[LW - r, r, -90],
			[LW - r, LH - r, 0],
			[r, LH - r, 90],
			[r, r, 180],
		];
		for (const [ccu, ccv, a0] of corners) {
			for (let k = 0; k <= 6; k++) {
				const a = ((a0 + (k * 90) / 6) * Math.PI) / 180;
				outline.push(ccu + r * Math.cos(a), ccv + r * Math.sin(a));
			}
		}
	}
	const outlinePath = (y: number) => {
		ctx.beginPath();
		for (let i = 0; i < outline.length; i += 2) (i ? ln : mv)(outline[i], outline[i + 1], y);
		ctx.closePath();
	};

	// gauge helpers, angle in degrees, counter clockwise, v points down
	const au = (r: number, a: number) => GU + r * Math.cos((a * Math.PI) / 180);
	const av = (r: number, a: number) => GV - r * Math.sin((a * Math.PI) / 180);
	const arc = (r: number, a0: number, a1: number, y: number, move: boolean) => {
		const n = Math.max(2, Math.ceil(Math.abs(a1 - a0) / 3));
		for (let k = 0; k <= n; k++) {
			const a = a0 + ((a1 - a0) * k) / n;
			(move && k === 0 ? mv : ln)(au(r, a), av(r, a), y);
		}
	};
	const sector = (a0: number, a1: number, y: number) => {
		ctx.beginPath();
		arc(R2, a0, a1, y, true);
		arc(R1, a1, a0, y, false);
		ctx.closePath();
	};

	// animated state
	let value = 0.5;
	let aVal = A0;
	const bars = new Float32Array(6);
	let digits = '';
	let segs: number[] = []; // lit 7 segment strokes u0 v0 u1 v1
	const ghost: number[] = [];
	let tipU = 0;
	let tipV = 0;
	let tailU = 0;
	let tailV = 0;

	const segLines = (d: number, mask: number, out: number[]) => {
		const x = DIG_X + d * DIG_S;
		const y = DIG_Y;
		const w = DIG_W;
		const h = DIG_H;
		const m = h / 2;
		const sk = (vv: number) => (h - vv) * 0.12; // italic
		const L: number[][] = [
			[x + 2, y, x + w - 2, y],
			[x + w, y + 2, x + w, y + m - 2],
			[x + w, y + m + 2, x + w, y + h - 2],
			[x + 2, y + h, x + w - 2, y + h],
			[x, y + m + 2, x, y + h - 2],
			[x, y + 2, x, y + m - 2],
			[x + 2, y + m, x + w - 2, y + m],
		];
		for (let s = 0; s < 7; s++) {
			if (!(mask & (1 << s))) continue;
			const [u0, v0, u1, v1] = L[s];
			out.push(u0 + sk(v0 - y), v0, u1 + sk(v1 - y), v1);
		}
	};
	for (let d = 0; d < 4; d++) segLines(d, 0x7f, ghost);

	const update = (t: number) => {
		value = 0.52 + 0.3 * Math.sin(t * 0.47) + 0.14 * Math.sin(t * 1.21 + 1.3);
		value = Math.min(0.98, Math.max(0.02, value));
		aVal = A0 - SWEEP * value;
		for (let i = 0; i < 6; i++) bars[i] = BAR_H * (0.55 + 0.4 * Math.sin(t * (0.8 + i * 0.27) + i * 1.9));
		const rpm = String(Math.round(value * 700) * 10);
		if (rpm !== digits) {
			digits = rpm;
			segs = [];
			const s = rpm.padStart(4, ' ');
			for (let d = 0; d < 4; d++) if (s[d] !== ' ') segLines(d, SEGS[Number(s[d])], segs);
		}
		tipU = au(R1 - 8, aVal);
		tipV = av(R1 - 8, aVal);
		tailU = au(-12, aVal);
		tailV = av(-12, aVal);
	};

	const segDist = (u: number, v: number, x0: number, y0: number, x1: number, y1: number) => {
		const dx = x1 - x0;
		const dy = y1 - y0;
		const l = dx * dx + dy * dy;
		let k = l ? ((u - x0) * dx + (v - y0) * dy) / l : 0;
		k = k < 0 ? 0 : k > 1 ? 1 : k;
		const ex = x0 + k * dx - u;
		const ey = y0 + k * dy - v;
		return Math.sqrt(ex * ex + ey * ey);
	};

	// which color a layer position has: 0 none, 1 accent, 2 white, 3 dim
	const sample = (u: number, v: number) => {
		const du = u - GU;
		const dv = v - GV;
		const r2 = du * du + dv * dv;
		if (r2 <= R2 * R2) {
			if (r2 <= 49) return 2;
			if (segDist(u, v, tailU, tailV, tipU, tipV) < 2.2) return 2;
			if (r2 >= R1 * R1) {
				let a = (Math.atan2(-dv, du) * 180) / Math.PI;
				if (a < -90) a += 360;
				if (a >= A0 - SWEEP && a <= A0) return a >= aVal ? 1 : 3;
			}
			return 0;
		}
		if (u >= DIG_X - 4 && u <= DIG_X + 3 * DIG_S + DIG_W + 6 && v >= DIG_Y - 3 && v <= DIG_Y + DIG_H + 3) {
			for (let i = 0; i < segs.length; i += 4) if (segDist(u, v, segs[i], segs[i + 1], segs[i + 2], segs[i + 3]) < 2.2) return 2;
			return 0;
		}
		if (u >= BAR_X && v <= BAR_B && v >= BAR_B - BAR_H - 20) {
			const i = Math.floor((u - BAR_X) / BAR_S);
			if (i < 6 && u - BAR_X - i * BAR_S <= BAR_W && v >= BAR_B - bars[i]) return 1;
		}
		return 0;
	};

	const COL = ['', ACCENT, WHITE, '#3b414b'];

	const layerFill = (y: number, top: boolean) => {
		outlinePath(y);
		ctx.fillStyle = top ? 'rgba(8, 10, 13, 0.94)' : 'rgba(8, 10, 13, 0.8)';
		ctx.fill();
		ctx.lineWidth = 1;
		ctx.strokeStyle = top ? 'rgba(255, 255, 255, 0.3)' : 'rgba(255, 255, 255, 0.14)';
		ctx.stroke();
	};

	// head: dot matrix
	const drawHead = (y: number) => {
		const step = 8;
		const paths: Path2D[] = [new Path2D(), new Path2D(), new Path2D(), new Path2D()];
		for (let v = step / 2; v < LH; v += step) {
			for (let u = step / 2; u < LW; u += step) {
				const c = sample(u, v);
				P(u, v, y);
				const s = (c ? 4.2 : 2.2) * pt[2];
				paths[c].rect(pt[0] - s / 2, pt[1] - s * 0.33, s, s * 0.66);
			}
		}
		ctx.fillStyle = 'rgba(255, 255, 255, 0.07)';
		ctx.fill(paths[0]);
		for (let c = 1; c < 4; c++) {
			ctx.fillStyle = COL[c];
			ctx.fill(paths[c]);
		}
	};

	// drv: horizontal runs, the way vic sends the interior of shapes to the controller
	const drawDrv = (y: number) => {
		const paths: Path2D[] = [new Path2D(), new Path2D(), new Path2D(), new Path2D()];
		for (let v = 3; v < LH; v += 4) {
			let run = 0;
			let u0 = 0;
			for (let u = 0; u <= LW; u++) {
				const c = u < LW ? sample(u, v) : 0;
				if (c !== run) {
					if (run) {
						P(u0, v, y);
						paths[run].moveTo(pt[0], pt[1]);
						P(u - 1, v, y);
						paths[run].lineTo(pt[0], pt[1]);
					}
					run = c;
					u0 = u;
				}
			}
		}
		ctx.lineWidth = 1.5;
		ctx.lineCap = 'butt';
		for (let c = 1; c < 4; c++) {
			ctx.strokeStyle = COL[c];
			ctx.stroke(paths[c]);
		}
	};

	const handle = (u: number, v: number, y: number) => {
		P(u, v, y);
		ctx.rect(pt[0] - 2, pt[1] - 2, 4, 4);
	};

	// gpr: wire frame of the primitives with their control points
	const drawGpr = (y: number) => {
		ctx.lineWidth = 1;
		ctx.strokeStyle = 'rgba(238, 240, 243, 0.55)';
		ctx.beginPath();
		arc(R2, A0, A0 - SWEEP, y, true);
		arc(R1, A0 - SWEEP, A0, y, false);
		ctx.closePath();
		arc(7, 0, 360, y, true);
		mv(tailU, tailV, y);
		ln(tipU, tipV, y);
		for (let i = 0; i < 6; i++) {
			const x = BAR_X + i * BAR_S;
			mv(x, BAR_B, y);
			ln(x, BAR_B - bars[i], y);
			ln(x + BAR_W, BAR_B - bars[i], y);
			ln(x + BAR_W, BAR_B, y);
		}
		for (let i = 0; i < segs.length; i += 4) {
			mv(segs[i], segs[i + 1], y);
			ln(segs[i + 2], segs[i + 3], y);
		}
		ctx.stroke();
		ctx.strokeStyle = ACCENT;
		ctx.beginPath();
		mv(au(R1, aVal), av(R1, aVal), y);
		ln(au(R2, aVal), av(R2, aVal), y);
		ctx.stroke();
		ctx.fillStyle = ACCENT;
		ctx.beginPath();
		handle(tipU, tipV, y);
		handle(GU, GV, y);
		handle(au(R2, A0), av(R2, A0), y);
		handle(au(R2, A0 - SWEEP), av(R2, A0 - SWEEP), y);
		for (let i = 0; i < 6; i++) handle(BAR_X + i * BAR_S, BAR_B - bars[i], y);
		ctx.fill();
	};

	// dc: the finished drawing
	const drawDc = (y: number) => {
		P(GU, GV, y);
		const k = pt[2];
		// ring
		sector(aVal, A0 - SWEEP, y);
		ctx.fillStyle = DIM;
		ctx.fill();
		sector(A0, aVal, y);
		ctx.fillStyle = ACCENT;
		ctx.shadowColor = 'rgba(56, 189, 248, 0.55)';
		ctx.shadowBlur = 14;
		ctx.fill();
		// bars
		ctx.beginPath();
		for (let i = 0; i < 6; i++) {
			const x = BAR_X + i * BAR_S;
			mv(x, BAR_B, y);
			ln(x, BAR_B - bars[i], y);
			ln(x + BAR_W, BAR_B - bars[i], y);
			ln(x + BAR_W, BAR_B, y);
			ctx.closePath();
		}
		ctx.fill();
		ctx.shadowBlur = 0;
		// ticks
		ctx.lineWidth = 1;
		ctx.strokeStyle = 'rgba(238, 240, 243, 0.5)';
		ctx.beginPath();
		for (let i = 0; i <= 27; i++) {
			const a = A0 - (SWEEP * i) / 27;
			const r = i % 3 === 0 ? 84 : 80;
			mv(au(76, a), av(76, a), y);
			ln(au(r, a), av(r, a), y);
		}
		ctx.stroke();
		// 7 segment readout
		ctx.lineCap = 'round';
		ctx.lineWidth = 3.4 * k;
		ctx.strokeStyle = '#14181d';
		ctx.beginPath();
		for (let i = 0; i < ghost.length; i += 4) {
			mv(ghost[i], ghost[i + 1], y);
			ln(ghost[i + 2], ghost[i + 3], y);
		}
		ctx.stroke();
		ctx.strokeStyle = WHITE;
		ctx.beginPath();
		for (let i = 0; i < segs.length; i += 4) {
			mv(segs[i], segs[i + 1], y);
			ln(segs[i + 2], segs[i + 3], y);
		}
		ctx.stroke();
		// needle and hub
		ctx.lineWidth = 2.6 * k;
		ctx.beginPath();
		mv(tailU, tailV, y);
		ln(tipU, tipV, y);
		ctx.stroke();
		ctx.fillStyle = WHITE;
		ctx.beginPath();
		arc(7, 0, 360, y, true);
		ctx.fill();
		ctx.fillStyle = '#0b0d10';
		ctx.beginPath();
		arc(2.5, 0, 360, y, true);
		ctx.fill();
		ctx.lineCap = 'butt';
	};

	const CORNERS = [
		[0, 0],
		[LW, 0],
		[LW, LH],
		[0, LH],
	];

	const frame = (t: number, open: number, yaw: number, pitch: number) => {
		update(t);
		cyaw = Math.cos(yaw);
		syaw = Math.sin(yaw);
		cp = Math.cos(pitch);
		sp = Math.sin(pitch);
		const gap = 6 + 84 * open;
		yOff = gap * 1.5;
		ctx.clearRect(0, 0, W, H);

		const draw = [drawHead, drawDrv, drawGpr, drawDc];
		for (let i = 0; i < 4; i++) {
			const y = i * gap;
			if (i) {
				// corner guides from the layer below
				ctx.setLineDash([2, 4]);
				ctx.lineWidth = 1;
				ctx.strokeStyle = 'rgba(255, 255, 255, 0.16)';
				ctx.beginPath();
				for (const [u, v] of CORNERS) {
					mv(u, v, y - gap);
					ln(u, v, y);
				}
				ctx.stroke();
				ctx.setLineDash([]);
			}
			layerFill(y, i === 3);
			draw[i](y);
		}

		// data packets falling through the layers
		const top = 3 * gap;
		const tracks = [
			[tipU, tipV],
			[BAR_X + 2 * BAR_S + BAR_W / 2, BAR_B - bars[2]],
			[DIG_X + DIG_S + DIG_W / 2, DIG_Y + DIG_H / 2],
		];
		ctx.lineWidth = 1.5;
		for (let i = 0; i < tracks.length && open > 0.6; i++) {
			const ph = (t * 0.45 + i / tracks.length) % 1;
			const y = top - ph * top;
			const [u, v] = tracks[i];
			ctx.globalAlpha = Math.sin(ph * Math.PI);
			ctx.strokeStyle = ACCENT;
			ctx.beginPath();
			mv(u, v, Math.min(top, y + 26));
			ln(u, v, y);
			ctx.stroke();
			P(u, v, y);
			ctx.fillStyle = '#fff';
			ctx.beginPath();
			ctx.arc(pt[0], pt[1], 2.2, 0, Math.PI * 2);
			ctx.fill();
		}
		ctx.globalAlpha = 1;

		// labels
		if (labels && open > 0.05) {
			ctx.globalAlpha = Math.min(1, open * 1.3);
			let lx = 0;
			const ys: number[] = [];
			for (let i = 0; i < 4; i++) {
				P(LW, LH / 2, i * gap);
				lx = Math.max(lx, pt[0]);
				ys.push(pt[1]);
			}
			lx += 26;
			for (let i = 0; i < 4; i++) {
				P(LW, LH / 2, i * gap);
				ctx.strokeStyle = 'rgba(255, 255, 255, 0.22)';
				ctx.lineWidth = 1;
				ctx.beginPath();
				ctx.moveTo(pt[0] + 6, pt[1]);
				ctx.lineTo(lx, pt[1]);
				ctx.stroke();
				ctx.fillStyle = i === 3 ? ACCENT : WHITE;
				ctx.beginPath();
				ctx.arc(lx, pt[1], 2.5, 0, Math.PI * 2);
				ctx.fill();
				ctx.textBaseline = 'middle';
				ctx.font = '600 13px ui-monospace, "Cascadia Code", Consolas, monospace';
				ctx.fillText(LAYERS[i].name, lx + 10, pt[1] - 8);
				ctx.fillStyle = '#8a8f99';
				ctx.font = '12px Inter, system-ui, "Segoe UI", sans-serif';
				ctx.fillText(LAYERS[i].desc, lx + 10, pt[1] + 9);
			}
			ctx.globalAlpha = 1;
		}
	};

	resize();
	let resizeTimer = 0;
	window.addEventListener('resize', () => {
		clearTimeout(resizeTimer);
		resizeTimer = window.setTimeout(resize, 100);
	});

	// pointer parallax
	let px = 0;
	let py = 0;
	let sx = 0;
	let sy = 0;
	if (!reduced && window.matchMedia('(pointer: fine)').matches) {
		window.addEventListener(
			'pointermove',
			(e) => {
				px = e.clientX / window.innerWidth - 0.5;
				py = e.clientY / window.innerHeight - 0.5;
			},
			{ passive: true },
		);
	}

	let visible = true;
	new IntersectionObserver(([e]) => (visible = e.isIntersecting)).observe(canvas);
	const t0 = performance.now();
	const loop = (now: number) => {
		requestAnimationFrame(loop);
		if (!visible || document.hidden) return;
		const t = (now - t0) / 1000;
		const k = Math.min(1, Math.max(0, (t - 0.4) / 1.8));
		const open = reduced ? 1 : 1 - Math.pow(1 - k, 3);
		sx += (px - sx) * 0.05;
		sy += (py - sy) * 0.05;
		const drift = reduced ? 0 : 0.07 * Math.sin(t * 0.21);
		frame(t, open, -0.5 + drift + sx * 0.3, 0.5 + sy * 0.1);
	};
	requestAnimationFrame(loop);
}

/* ------------------------------------------------------------------------------------------------ */
function reveals(): void {
	const els = document.querySelectorAll<HTMLElement>('.reveal');
	const counters = document.querySelectorAll<HTMLElement>('[data-count]');
	if (!('IntersectionObserver' in window)) {
		els.forEach((e) => e.classList.add('in'));
		return;
	}
	const io = new IntersectionObserver(
		(entries) => {
			for (const e of entries) {
				if (!e.isIntersecting) continue;
				e.target.classList.add('in');
				io.unobserve(e.target);
				e.target.querySelectorAll<HTMLElement>('[data-count]').forEach(countUp);
			}
		},
		{ threshold: 0.15, rootMargin: '0px 0px -40px 0px' },
	);
	els.forEach((e) => io.observe(e));
	counters.forEach((c) => (c.textContent = '0'));
}

function countUp(el: HTMLElement): void {
	const target = Number(el.dataset.count || '0');
	if (!target) {
		el.textContent = '0';
		return;
	}
	const t0 = performance.now();
	const dur = 1200;
	const tick = (t: number) => {
		const k = Math.min(1, (t - t0) / dur);
		el.textContent = String(Math.round(target * (1 - Math.pow(1 - k, 3))));
		if (k < 1) requestAnimationFrame(tick);
	};
	requestAnimationFrame(tick);
}

function spotlight(): void {
	document.querySelectorAll<HTMLElement>('.card').forEach((card) => {
		card.addEventListener('pointermove', (e) => {
			const r = card.getBoundingClientRect();
			card.style.setProperty('--mx', `${e.clientX - r.left}px`);
			card.style.setProperty('--my', `${e.clientY - r.top}px`);
		});
		card.addEventListener('pointerleave', () => {
			card.style.removeProperty('--mx');
			card.style.removeProperty('--my');
		});
	});
}

function magnifier(): void {
	const box = document.getElementById('magnify');
	const img = box?.querySelector('img');
	const lens = box?.querySelector<HTMLElement>('.lens');
	if (!box || !img || !lens) return;
	const ZOOM = 3;
	const move = (e: PointerEvent) => {
		const ir = img.getBoundingClientRect();
		const br = box.getBoundingClientRect();
		const x = e.clientX - ir.left;
		const y = e.clientY - ir.top;
		if (x < 0 || y < 0 || x > ir.width || y > ir.height) {
			box.classList.remove('active');
			return;
		}
		box.classList.add('active');
		const size = lens.offsetWidth;
		lens.style.left = `${e.clientX - br.left - size / 2}px`;
		lens.style.top = `${e.clientY - br.top - size / 2}px`;
		lens.style.backgroundSize = `${ir.width * ZOOM}px ${ir.height * ZOOM}px`;
		lens.style.backgroundPosition = `${-(x * ZOOM - size / 2)}px ${-(y * ZOOM - size / 2)}px`;
	};
	box.addEventListener('pointermove', move);
	box.addEventListener('pointerdown', move);
	box.addEventListener('pointerleave', () => box.classList.remove('active'));
}

function tabs(): void {
	const root = document.getElementById('tabs');
	if (!root) return;
	const buttons = root.querySelectorAll<HTMLButtonElement>('[data-tab]');
	const panels = root.querySelectorAll<HTMLElement>('[data-panel]');
	buttons.forEach((b) =>
		b.addEventListener('click', () => {
			buttons.forEach((o) => o.setAttribute('aria-selected', String(o === b)));
			panels.forEach((p) => (p.hidden = p.dataset.panel !== b.dataset.tab));
		}),
	);
}
