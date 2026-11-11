// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import { existsSync } from 'node:fs';

// sidebar entry for a docs page, skipped with a warning while the page doesn't exist
const docs = new URL('./src/content/docs/', import.meta.url);
const page = (slug) => {
	if (existsSync(new URL(`${slug}.md`, docs)) || existsSync(new URL(`${slug}/index.md`, docs))) return [{ slug }];
	console.warn(`[sidebar] missing page: ${slug}`);
	return [];
};

// https://astro.build/config
export default defineConfig({
	site: 'https://mpaland.github.io',
	base: '/vic',
	trailingSlash: 'always',
	integrations: [
		starlight({
			title: 'vic',
			description:
				'Header only C++ graphics library for graphic, dot matrix, alpha numeric and seven-segment displays in embedded systems',
			logo: { src: './src/assets/logo.svg', replacesTitle: false },
			favicon: '/favicon.svg',
			social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/mpaland/vic' }],
			editLink: { baseUrl: 'https://github.com/mpaland/vic/edit/master/site/' },
			customCss: ['./src/styles/fonts.css', './src/styles/theme.css'],
			components: {
				ThemeProvider: './src/components/overrides/ThemeProvider.astro',
				ThemeSelect: './src/components/overrides/ThemeSelect.astro',
			},
			expressiveCode: { themes: ['github-dark-default'] },
			sidebar: [
				{ label: 'Getting started', items: [{ autogenerate: { directory: 'getting-started' } }] },
				{ label: 'Design', items: [{ autogenerate: { directory: 'design' } }] },
				{
					label: 'Command reference',
					items: [
						...page('reference'),
						...page('reference/dc'),
						...page('reference/tc'),
						...page('reference/drv'),
						...page('reference/sprite'),
						...page('reference/shader'),
						{ label: 'Widgets', collapsed: true, items: [{ autogenerate: { directory: 'reference/widgets' } }] },
						...page('reference/color-util'),
						...page('reference/fonts'),
						...page('reference/config'),
					],
				},
				{ label: 'Heads', collapsed: true, items: [{ autogenerate: { directory: 'heads' } }] },
				{ label: 'Examples', items: [{ autogenerate: { directory: 'examples' } }] },
				...page('changelog'),
			],
		}),
	],
});
