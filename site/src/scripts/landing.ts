// Landing page interactions: pixel canvas, device tilt and demo cycling, reveals, magnifier, tabs.

const reduced = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

export function initLanding(): void {
	navShadow();
	pixelCanvas();
	device();
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
 * Background pixel canvas: a low resolution "display" on which lines, circles, boxes and gauge
 * sweeps are plotted pixel by pixel with Bresenham / midpoint routines and fade out slowly.
 * This is a page effect in the spirit of vic, not vic output.
 */
type Shape = { pts: Int32Array; n: number; i: number; speed: number; rgb: [number, number, number]; needle?: () => number[] };

const PALETTE: [number, number, number][] = [
	[34, 227, 255],
	[139, 92, 246],
	[255, 79, 216],
	[255, 181, 71],
	[57, 255, 158],
];

function pixelCanvas(): void {
	const canvas = document.getElementById('pix') as HTMLCanvasElement | null;
	const hero = document.getElementById('hero');
	if (!canvas || !hero) return;
	const ctx = canvas.getContext('2d', { alpha: false });
	if (!ctx) return;

	const CELL = 5;
	let W = 0;
	let H = 0;
	let buf = new Float32Array(0);
	let img: ImageData;
	let shapes: Shape[] = [];

	const resize = () => {
		W = Math.max(40, Math.ceil(hero.clientWidth / CELL));
		H = Math.max(30, Math.ceil(hero.clientHeight / CELL));
		canvas.width = W;
		canvas.height = H;
		buf = new Float32Array(W * H * 3);
		img = ctx.createImageData(W, H);
		for (let i = 3; i < img.data.length; i += 4) img.data[i] = 255;
		shapes = [];
	};

	const rnd = (a: number, b: number) => a + Math.floor(Math.random() * (b - a + 1));

	const line = (x0: number, y0: number, x1: number, y1: number, out: number[]) => {
		const dx = Math.abs(x1 - x0);
		const dy = -Math.abs(y1 - y0);
		const sx = x0 < x1 ? 1 : -1;
		const sy = y0 < y1 ? 1 : -1;
		let err = dx + dy;
		for (;;) {
			out.push(x0, y0);
			if (x0 === x1 && y0 === y1) break;
			const e2 = 2 * err;
			if (e2 >= dy) {
				err += dy;
				x0 += sx;
			}
			if (e2 <= dx) {
				err += dx;
				y0 += sy;
			}
		}
		return out;
	};

	// midpoint circle, points ordered by angle so it can be drawn as a sweep
	const circle = (cx: number, cy: number, r: number, a0 = 0, a1 = 360) => {
		const oct: [number, number][] = [];
		let x = r;
		let y = 0;
		let d = 1 - r;
		while (x >= y) {
			oct.push([x, y]);
			y++;
			if (d < 0) d += 2 * y + 1;
			else {
				x--;
				d += 2 * (y - x) + 1;
			}
		}
		const all: [number, number, number][] = [];
		for (const [px, py] of oct) {
			for (const [qx, qy] of [
				[px, py], [py, px], [-py, px], [-px, py], [-px, -py], [-py, -px], [py, -px], [px, -py],
			]) {
				let a = (Math.atan2(-qy, qx) * 180) / Math.PI;
				if (a < 0) a += 360;
				if (a >= a0 && a < a1) all.push([a, cx + qx, cy + qy]);
			}
		}
		all.sort((p, q) => p[0] - q[0]);
		const out: number[] = [];
		for (const p of all) out.push(p[1], p[2]);
		return out;
	};

	const spawn = () => {
		const rgb = PALETTE[rnd(0, PALETTE.length - 1)];
		const kind = rnd(0, 9);
		let pts: number[] = [];
		let needle: (() => number[]) | undefined;
		let speed = 3;
		if (kind <= 2) {
			// line burst from a point
			const x = rnd(0, W - 1);
			const y = rnd(0, H - 1);
			const n = rnd(3, 7);
			for (let k = 0; k < n; k++) line(x, y, x + rnd(-40, 40), y + rnd(-30, 30), pts);
			speed = 6;
		} else if (kind <= 4) {
			pts = circle(rnd(10, W - 10), rnd(8, H - 8), rnd(5, 22));
			speed = 3;
		} else if (kind <= 6) {
			// gauge: arc sweep with a needle that follows the sweep
			const cx = rnd(14, W - 14);
			const cy = rnd(12, H - 12);
			const r = rnd(8, 18);
			const start = rnd(0, 359);
			const arc = circle(cx, cy, r + 2, 0, 360);
			const rot = arc.length / 2;
			const k0 = Math.floor((start / 360) * rot);
			const sweep: number[] = [];
			for (let k = 0; k < rot * 0.75; k++) {
				const j = ((k0 + k) % rot) * 2;
				sweep.push(arc[j], arc[j + 1]);
			}
			pts = sweep;
			let step = 0;
			needle = () => {
				const j = Math.min(step, sweep.length / 2 - 1) * 2;
				step += 1;
				return line(cx, cy, sweep[j], sweep[j + 1], []);
			};
			speed = 1;
		} else if (kind <= 8) {
			const x = rnd(0, W - 30);
			const y = rnd(0, H - 20);
			const w = rnd(10, 46);
			const h = rnd(6, 26);
			line(x, y, x + w, y, pts);
			line(x + w, y, x + w, y + h, pts);
			line(x + w, y + h, x, y + h, pts);
			line(x, y + h, x, y, pts);
			speed = 4;
		} else {
			// sine trace like a scope
			const y0 = rnd(10, H - 10);
			const amp = rnd(3, 10);
			const per = rnd(16, 40);
			let py = y0;
			for (let x = 0; x < W; x++) {
				const y = Math.round(y0 + amp * Math.sin((x / per) * Math.PI * 2));
				line(x - 1 < 0 ? 0 : x - 1, py, x, y, pts);
				py = y;
			}
			speed = 8;
		}
		shapes.push({ pts: Int32Array.from(pts), n: pts.length / 2, i: 0, speed, rgb, needle });
	};

	const plot = (x: number, y: number, rgb: [number, number, number], k = 1) => {
		if (x < 0 || y < 0 || x >= W || y >= H) return;
		const o = (y * W + x) * 3;
		buf[o] = Math.max(buf[o], rgb[0] * k);
		buf[o + 1] = Math.max(buf[o + 1], rgb[1] * k);
		buf[o + 2] = Math.max(buf[o + 2], rgb[2] * k);
	};

	const step = () => {
		// fade
		for (let i = 0; i < buf.length; i++) buf[i] *= 0.978;
		// grow shapes
		for (const s of shapes) {
			const end = Math.min(s.n, s.i + s.speed);
			for (; s.i < end; s.i++) plot(s.pts[2 * s.i], s.pts[2 * s.i + 1], s.rgb);
			if (s.i < s.n) {
				// bright head
				plot(s.pts[2 * (s.i - 1)], s.pts[2 * (s.i - 1) + 1], [255, 255, 255]);
			}
			if (s.needle && s.i < s.n) {
				const p = s.needle();
				for (let k = 0; k < p.length; k += 2) plot(p[k], p[k + 1], s.rgb, 0.45);
			}
		}
		shapes = shapes.filter((s) => s.i < s.n);
		if (shapes.length < 7 && Math.random() < 0.08) spawn();
	};

	const draw = () => {
		const d = img.data;
		for (let i = 0, j = 0; i < buf.length; i += 3, j += 4) {
			d[j] = buf[i];
			d[j + 1] = buf[i + 1];
			d[j + 2] = buf[i + 2];
		}
		ctx.putImageData(img, 0, 0);
	};

	resize();
	let resizeTimer = 0;
	window.addEventListener('resize', () => {
		clearTimeout(resizeTimer);
		resizeTimer = window.setTimeout(resize, 150);
	});

	for (let k = 0; k < 4; k++) spawn();
	let visible = true;
	new IntersectionObserver(([e]) => (visible = e.isIntersecting)).observe(hero);
	let last = 0;
	const loop = (t: number) => {
		requestAnimationFrame(loop);
		if (!visible || document.hidden || t - last < 33) return;
		last = t;
		step();
		draw();
	};
	requestAnimationFrame(loop);
}

/* ------------------------------------------------------------------------------------------------
 * Device: pointer tilt and demo cycling with a scanline wipe.
 */
function device(): void {
	const hero = document.getElementById('hero');
	const dev = document.getElementById('device');
	const screen = document.getElementById('screen');
	const label = document.getElementById('now');
	const data = document.getElementById('hero-demos');
	if (!hero || !dev || !screen || !label || !data) return;

	if (!reduced && window.matchMedia('(pointer: fine)').matches) {
		hero.addEventListener('pointermove', (e) => {
			const r = hero.getBoundingClientRect();
			const x = (e.clientX - r.left) / r.width - 0.5;
			const y = (e.clientY - r.top) / r.height - 0.5;
			dev.style.setProperty('--ry', `${-14 + x * 18}deg`);
			dev.style.setProperty('--rx', `${6 - y * 12}deg`);
		});
		hero.addEventListener('pointerleave', () => {
			dev.style.removeProperty('--ry');
			dev.style.removeProperty('--rx');
		});
	}

	const demos: { src: string; label: string }[] = JSON.parse(data.textContent || '[]');
	const layers = Array.from(screen.querySelectorAll<HTMLImageElement>('img.layer'));
	if (demos.length < 2 || layers.length < 2) return;

	let idx = 0;
	let cur = layers[0];
	let inc = layers[1];
	let busy = false;
	let visible = true;
	new IntersectionObserver(([e]) => (visible = e.isIntersecting)).observe(screen);

	const next = async () => {
		if (busy || !visible || document.hidden) return;
		busy = true;
		idx = (idx + 1) % demos.length;
		inc.src = demos[idx].src;
		inc.alt = `vic ${demos[idx].label} demo`;
		try {
			await inc.decode();
		} catch {
			/* show it anyway */
		}
		label.style.opacity = '0';
		inc.classList.add('incoming');
		screen.classList.add('wiping');
		window.setTimeout(() => {
			cur.classList.remove('show', 'boot');
			cur.removeAttribute('src');
			inc.classList.remove('incoming');
			inc.classList.add('show');
			screen.classList.remove('wiping');
			label.textContent = demos[idx].label;
			label.style.opacity = '1';
			[cur, inc] = [inc, cur];
			busy = false;
		}, 920);
	};
	window.setInterval(next, 7000);
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
