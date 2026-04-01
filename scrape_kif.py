"""
ShogiDB2 KIF Scraper
====================
Downloads KIF game records from shogidb2.com for building an opening book.

This script works in two phases:
  Phase 1 - Collect game URLs from the "popular games" listing pages using
            plain HTTP requests + HTML parsing (BeautifulSoup).
  Phase 2 - Visit each game page with a headless browser (Playwright), click
            the "KIF形式" export button, and save the KIF text to a file.

A headless browser is needed because shogidb2.com is built with Phoenix
LiveView — the KIF data isn't in the initial HTML. It only appears after a
JavaScript-driven interaction (clicking the export button triggers a websocket
message, and the server responds with the KIF text injected into a modal).

Dependencies (install these first):
    pip install requests beautifulsoup4 playwright
    playwright install chromium
"""

import os
import re
import sys
import time
import requests
from bs4 import BeautifulSoup
from playwright.sync_api import sync_playwright

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

BASE_URL = "https://shogidb2.com"
LISTING_URL = BASE_URL + "/tournament/アマ棋戦?q=&page={page}"
OUTPUT_DIR = "games/tournaments/アマ棋戦"

# How many listing pages to scrape (each page has ~20 games).
# Start small while testing, increase once you're confident it works.
NUM_PAGES = 85

# Seconds to wait between HTTP requests to be polite to the server.
REQUEST_DELAY = 0.5


# ---------------------------------------------------------------------------
# Phase 1: Collect game URLs from listing pages
# ---------------------------------------------------------------------------
#
# HOW THIS WORKS
# ==============
# The listing page at https://shogidb2.com/popular?q=&page=1 contains a list
# of games, each wrapped in an <a> tag whose href looks like:
#     /games/87a8f10ff41776...
#
# We use the `requests` library to download the page HTML and `BeautifulSoup`
# to parse it and find all links matching that pattern.
#
# requests  — Makes HTTP requests (GET, POST, etc.), like a browser visiting
#             a URL, but returns the raw HTML text instead of rendering it.
# BeautifulSoup — Parses an HTML string into a tree structure you can query
#                 with methods like find_all(), select(), etc.
# ---------------------------------------------------------------------------

def collect_game_urls(num_pages: int) -> list[str]:
    """
    Scrape the listing pages and return a list of full game URLs.
    """
    game_urls = []

    for page in range(1, num_pages + 1):
        url = LISTING_URL.format(page=page)
        print(f"[Phase 1] Fetching listing page {page}: {url}")

        # requests.get() sends an HTTP GET request — the same thing your
        # browser does when you type a URL and hit Enter.
        response = requests.get(url)

        # response.status_code tells us if the request succeeded (200 = OK).
        if response.status_code != 200:
            print(f"  WARNING: Got status {response.status_code}, skipping.")
            continue

        # Parse the HTML text into a BeautifulSoup object we can search.
        # "html.parser" is Python's built-in HTML parser (no extra install).
        soup = BeautifulSoup(response.text, "html.parser")

        # Find all <a> tags whose href starts with "/games/".
        # BeautifulSoup's find_all() searches the parsed HTML tree.
        # We pass a compiled regex to match only game links.
        game_links = soup.find_all("a", href=re.compile(r"^/games/"))

        for link in game_links:
            game_path = link["href"]          # e.g. "/games/87a8f10f..."
            full_url = BASE_URL + game_path   # e.g. "https://shogidb2.com/games/87a8f10f..."
            if full_url not in game_urls:
                game_urls.append(full_url)

        print(f"  Found {len(game_links)} game links (total unique so far: {len(game_urls)})")

        # Be polite: don't hammer the server with rapid-fire requests.
        time.sleep(REQUEST_DELAY)

    return game_urls


# ---------------------------------------------------------------------------
# Phase 2: Download KIF from each game page using a headless browser
# ---------------------------------------------------------------------------
#
# HOW THIS WORKS
# ==============
# shogidb2.com uses Phoenix LiveView, a framework where the page is rendered
# on the server and updated via a persistent websocket connection. Clicking
# the "KIF形式" button sends an event over the websocket; the server responds
# with the KIF text and injects it into a <textarea> inside a modal dialog.
#
# Because this interaction requires JavaScript + websockets, a plain HTTP GET
# won't give us the KIF. We need a real (headless) browser.
#
# Playwright — A library that launches and controls a real browser (Chromium,
#              Firefox, or WebKit) programmatically. "Headless" means no
#              visible window — it runs in the background.
#
# The workflow for each game:
#   1. Navigate to the game URL
#   2. Wait for the page to fully load (LiveView needs to connect its websocket)
#   3. Click the "KIF形式" button (has attribute phx-click="kif")
#   4. Wait for the modal dialog (#kifu-modal) to appear with text
#   5. Read the KIF text from the <textarea> inside the modal
#   6. Save it to a .kif file
# ---------------------------------------------------------------------------

AD_DOMAIN_PATTERNS = [
    "*googlesyndication.com*",
    "*googleadservices.com*",
    "*doubleclick.net*",
    "*google-analytics.com*",
    "*googletagmanager.com*",
    "*amazon-adsystem.com*",
]


def download_kifs(game_urls: list[str], output_dir: str):
    """
    Use Playwright to visit each game page, click the KIF button, and save
    the resulting KIF text to a file.
    """
    os.makedirs(output_dir, exist_ok=True)

    # sync_playwright() gives us a context manager that starts/stops the
    # Playwright server process managing the browser.
    with sync_playwright() as pw:

        # Launch a Chromium browser in headless mode (no visible window).
        browser = pw.chromium.launch(headless=True)

        # A browser context is like an incognito session — isolated cookies,
        # cache, etc. Useful if you wanted multiple parallel sessions.
        context = browser.new_context(viewport={"width": 1280, "height": 900})

        # Block ad/tracker requests so their iframes don't load.  Each
        # pattern is registered separately so only matching URLs are
        # intercepted — the site's own requests are never touched.
        for pattern in AD_DOMAIN_PATTERNS:
            context.route(pattern, lambda route: route.abort())

        for i, game_url in enumerate(game_urls, start=1):
            # Extract the game hash from the URL to use as a filename.
            game_hash = game_url.split("/games/")[-1]
            output_path = os.path.join(output_dir, f"{game_hash}.kif")

            if os.path.exists(output_path):
                print(f"[Phase 2] ({i}/{len(game_urls)}) Already saved, skipping: {game_hash[:20]}...")
                continue

            print(f"[Phase 2] ({i}/{len(game_urls)}) Visiting: {game_hash[:20]}...")

            # Use a fresh page (tab) per game so LiveView websocket state
            # from a previous game never leaks into the next one.
            page = context.new_page()

            try:
                page.goto(game_url, wait_until="networkidle", timeout=30000)

                # LiveView starts in "phx-loading" and transitions to
                # "phx-connected" once its websocket is established and
                # event handlers are bound.  Without this wait, JS clicks
                # on phx-click elements are silently ignored.
                # On 404 pages the KIF button doesn't exist, so we also
                # check for that and skip gracefully.
                try:
                    page.wait_for_selector(".phx-connected", timeout=10000)
                except Exception:
                    pass

                kif_btn = page.locator('[phx-click="kif"]')
                if kif_btn.count() == 0:
                    print(f"  Page not found (no KIF button), skipping.")
                    continue

                # Trigger the KIF export via JavaScript's native .click().
                # We can't use Playwright's page.click() because:
                #  - Ad iframes and the fixed navbar overlap the button,
                #    causing Playwright's hit-test to fail (30s timeout).
                #  - Playwright's force=True and dispatch_event("click")
                #    both fire low-level CDP events that don't propagate
                #    through Phoenix LiveView's event delegation system.
                # A JS .click() creates a trusted DOM event that LiveView
                # correctly captures and forwards over its websocket.
                page.evaluate(
                    '() => document.querySelector("[phx-click=kif]").click()'
                )

                # Wait for the textarea to exist in the DOM, then poll for
                # content.  We use state="attached" instead of "visible"
                # because the modal's CSS transition can leave the textarea
                # in a state Playwright doesn't consider "visible" even
                # though it is functionally present and being filled.
                textarea = page.locator("#kifu-modal textarea")
                textarea.wait_for(state="attached", timeout=10000)

                # Poll until the textarea has content (LiveView fills it
                # asynchronously after the click event).
                kif_text = ""
                for _ in range(20):
                    kif_text = textarea.input_value()
                    if kif_text.strip():
                        break
                    page.wait_for_timeout(500)

                if not kif_text.strip():
                    print(f"  WARNING: No KIF text found, skipping.")
                    continue

                with open(output_path, "w", encoding="utf-8") as f:
                    f.write(kif_text)

                print(f"  Saved: {output_path}")

            except Exception as e:
                print(f"  ERROR: {e}")
            finally:
                page.close()

            time.sleep(REQUEST_DELAY)

        browser.close()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    print("=" * 60)
    print("ShogiDB2 KIF Scraper")
    print("=" * 60)
    print()

    # Phase 1: Collect game URLs
    print("--- Phase 1: Collecting game URLs from listing pages ---")
    game_urls = collect_game_urls(NUM_PAGES)
    print(f"\nCollected {len(game_urls)} game URLs total.\n")

    if not game_urls:
        print("No games found. Exiting.")
        sys.exit(1)

    # Phase 2: Download KIF for each game
    print("--- Phase 2: Downloading KIF files ---")
    download_kifs(game_urls, OUTPUT_DIR)

    print(f"\nDone! KIF files saved to: {OUTPUT_DIR}/")


if __name__ == "__main__":
    main()
