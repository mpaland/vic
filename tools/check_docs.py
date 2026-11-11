#!/usr/bin/env python3
"""Checks that every head and widget header has a page on the website and vice versa.

  src/head/<Name>.h      -> site/src/content/docs/heads/<name>.md             (lower case, '_' -> '-')
  src/widget/<name>.h    -> site/src/content/docs/reference/widgets/<name>.md

Run from anywhere: python tools/check_docs.py
Exit code 1 if a page is missing or a page has no header (orphan).
The pages themselves are compared with the headers by hand before every release.
"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / 'site' / 'src' / 'content' / 'docs'

# header dir, docs dir, headers without a page, pages without a header
CHECKS = [
    ('src/head', 'heads', set(), {'index'}),
    ('src/widget', 'reference/widgets', {'widget'}, {'index'}),
]


def slug(stem: str) -> str:
    return stem.lower().replace('_', '-')


def main() -> int:
    errors = 0
    for hdr_dir, doc_dir, no_page, no_header in CHECKS:
        headers = {slug(p.stem): p.name for p in (ROOT / hdr_dir).glob('*.h') if p.stem not in no_page}
        pages = {p.stem for p in (DOCS / doc_dir).glob('*.md*')} - no_header
        for s in sorted(set(headers) - pages):
            print(f'missing page: {hdr_dir}/{headers[s]} -> site/src/content/docs/{doc_dir}/{s}.md')
            errors += 1
        for s in sorted(pages - set(headers)):
            print(f'orphan page:  site/src/content/docs/{doc_dir}/{s}.md has no header in {hdr_dir}/')
            errors += 1
        print(f'{hdr_dir}: {len(headers)} headers, {len(pages)} pages')
    if errors:
        print(f'check_docs: {errors} problem(s)')
        return 1
    print('check_docs: ok')
    return 0


if __name__ == '__main__':
    sys.exit(main())
