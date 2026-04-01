"use strict";

/*
  Startup hints runtime model:
  1) This host page controls navigation and title (index shell).
  2) Host theme CSS is applied here (light.css / dark.css).
  3) Each iframe page applies its own theme through theme-loader.js
     using the ?theme=... query param.
*/

// ─── Config ──────────────────────────────────────────────────────────────────

var CONFIG = {
  language:    "en",
  localesRoot: "locales",
  maxHints:    50
};

var DEBUG = new URLSearchParams(window.location.search).get("debug") === "1";

function debugLog() {
  if (!DEBUG) return;
  var args = Array.prototype.slice.call(arguments);
  args.unshift("[startupHints]");
  console.log.apply(console, args);
}

var THEME_SYNC_RETRY_INTERVAL_MS = 100;
var THEME_SYNC_MAX_ATTEMPTS = 40;

// ─── State ───────────────────────────────────────────────────────────────────

var state = {
  currentIndex: 1,
  fileCount:    0,
  theme:        "light"
};

// ─── DOM cache ───────────────────────────────────────────────────────────────

var dom = {
  source:         null,
  left:           null,
  right:          null,
  titleSpan:      null
};

function cacheDomElements() {
  dom.source         = document.getElementById("source");
  dom.left           = document.getElementById("left");
  dom.right          = document.getElementById("right");
  dom.titleSpan      = document.querySelector("#title span");
}

// ─── URL builders ────────────────────────────────────────────────────────────

function buildHintFileUrl(index) {
  return CONFIG.localesRoot + "/" + CONFIG.language + "/" + index + ".html";
}

function buildHintUrl(index) {
  return buildHintFileUrl(index) + "?theme=" + encodeURIComponent(state.theme);
}

function buildErrorUrl() {
  return CONFIG.localesRoot + "/" + CONFIG.language + "/error.html"
       + "?theme=" + encodeURIComponent(state.theme);
}

// ─── Language detection ─────────────────────────────────────────────────────

function normalizeLanguage(value) {
  if (typeof value !== "string") return null;
  var trimmed = value.trim();
  if (!trimmed) return null;

  // Accept forms like "de", "de-DE", "de_DE" and keep the base language.
  var base = trimmed.split(/[-_]/)[0].toLowerCase();
  return /^[a-z]{2,3}$/.test(base) ? base : null;
}

function detectLanguageFromUrl() {
  var params = new URLSearchParams(window.location.search);
  return normalizeLanguage(params.get("lang") || params.get("language"));
}

function detectLanguageFromSankore() {
  if (!window.sankore) return null;

  try {
    if (typeof window.sankore.lang === "function") {
      return normalizeLanguage(window.sankore.lang());
    }

    return normalizeLanguage(window.sankore.lang);
  } catch (e) {
    return null;
  }
}

function getCurrentLanguage() {
  return detectLanguageFromUrl()
      || detectLanguageFromSankore()
      || normalizeLanguage(CONFIG.language)
      || "en";
}

function localeExists(language) {
  return requestText(CONFIG.localesRoot + "/" + language + "/1.html")
    .then(function() { return true; })
    .catch(function() { return false; });
}

async function resolveLanguage(preferredLanguage) {
  var candidates = [
    preferredLanguage,
    normalizeLanguage(CONFIG.language),
    "en"
  ].filter(Boolean);

  var seen = Object.create(null);
  for (var i = 0; i < candidates.length; i++) {
    var candidate = candidates[i];
    if (seen[candidate]) continue;
    seen[candidate] = true;

    if (await localeExists(candidate)) return candidate;
  }

  return "en";
}

// ─── Theme detection ─────────────────────────────────────────────────────────

function normalizeTheme(value) {
  if (value === "dark" || value === "light") return value;
  return null;
}

function detectThemeFromUrl() {
  var params = new URLSearchParams(window.location.search);
  return normalizeTheme(params.get("theme"));
}

function detectThemeFromSankore() {
  if (window.sankore && typeof window.sankore.isDarkMode !== "undefined") {
    return window.sankore.isDarkMode ? "dark" : "light";
  }
  return null;
}

function resolveTheme() {
  return detectThemeFromUrl() || detectThemeFromSankore() || state.theme || "light";
}

// ─── Theme application ───────────────────────────────────────────────────────

// Host-only theme stylesheet.
// The iframe theme is handled by theme-loader.js.

function applyHostThemeStylesheet(theme) {
  var safeTheme = normalizeTheme(theme) || "light";
  var themeUrl  = new URL("css/" + safeTheme + ".css", window.location.href).href;
  var linkId    = "ob-theme-css";
  var link      = document.getElementById(linkId);

  if (!link) {
    link      = document.createElement("link");
    link.id   = linkId;
    link.rel  = "stylesheet";
    link.type = "text/css";
    document.head.appendChild(link);
  }

  if (link.href !== themeUrl) link.href = themeUrl;
}

function applyHostTheme(theme) {
  var safeTheme = normalizeTheme(theme) || "light";
  document.documentElement.setAttribute("data-theme", safeTheme);
  applyHostThemeStylesheet(safeTheme);
}

// Reload the current iframe page with the updated ?theme value.
function refreshCurrentHintTheme() {
  if (!dom.source || state.fileCount < 1) return;
  dom.source.src = buildHintUrl(state.currentIndex);
}

// Sync host + iframe when OpenBoard theme changes.
function bindThemeSync() {
  function connectThemeBridge() {
    if (!window.sankore
      || !window.sankore.themeChanged
      || typeof window.sankore.themeChanged.connect !== "function") {
      return false;
    }

    window.sankore.themeChanged.connect(function(isDark) {
      state.theme = isDark ? "dark" : "light";
      applyHostTheme(state.theme);
      // The iframe theme is applied by theme-loader.js via ?theme=...
      refreshCurrentHintTheme();
    });

    var current = detectThemeFromSankore();
    if (current && current !== state.theme) {
      state.theme = current;
      applyHostTheme(state.theme);
    }

    return true;
  }

  if (connectThemeBridge()) return;

  var attempts = 0;
  var timer = setInterval(function() {
    if (connectThemeBridge() || ++attempts >= THEME_SYNC_MAX_ATTEMPTS) {
      clearInterval(timer);
    }
  }, THEME_SYNC_RETRY_INTERVAL_MS);
}

// ─── Navigation ──────────────────────────────────────────────────────────────

function loadCurrentHint() {
  if (!dom.source) return;
  state.theme      = resolveTheme();
  applyHostTheme(state.theme);
  dom.source.src   = buildHintUrl(state.currentIndex);
  debugLog("loadHint", {
    index: state.currentIndex,
    fileCount: state.fileCount,
    language: CONFIG.language,
    theme: state.theme,
    src: dom.source.src
  });
}

function goToNextHint() {
  if (state.fileCount < 1) return;
  state.currentIndex = state.currentIndex < state.fileCount
    ? state.currentIndex + 1
    : 1;
  loadCurrentHint();
}

function goToPreviousHint() {
  if (state.fileCount < 1) return;
  state.currentIndex = state.currentIndex > 1
    ? state.currentIndex - 1
    : state.fileCount;
  loadCurrentHint();
}

function bindNavigationHandlers() {
  if (dom.right) dom.right.addEventListener("click", goToNextHint);
  if (dom.left)  dom.left.addEventListener("click", goToPreviousHint);

  document.addEventListener("keydown", function(e) {
    if (e.key === "ArrowRight") goToNextHint();
    if (e.key === "ArrowLeft")  goToPreviousHint();
  });
}

// ─── Iframe handlers ─────────────────────────────────────────────────────────

function bindFrameHandlers() {
  if (!dom.source) return;
  dom.source.addEventListener("load", setTitle);
  dom.source.addEventListener("load", function() {
    debugLog("iframeLoaded", {
      index: state.currentIndex,
      src: dom.source ? dom.source.src : null
    });
  });
}

// ─── Hint detection ──────────────────────────────────────────────────────────

// Probes files sequentially until the first missing page is found.
async function detectAvailableHints() {
  while (state.fileCount < CONFIG.maxHints) {
    try {
      await requestText(buildHintFileUrl(state.fileCount + 1));
      state.fileCount++;
    } catch (e) {
      break;
    }
  }

  if (state.fileCount === CONFIG.maxHints) {
    console.warn("Hint detection reached maxHints limit:", CONFIG.maxHints);
  }
}

// ─── HTTP helper ─────────────────────────────────────────────────────────────

// Tries fetch first, falls back to XHR for file:// contexts (OpenBoard).
function requestText(url) {
  return fetch(url, { method: "GET", cache: "no-store" })
    .then(function(response) {
      if (!response.ok) throw new Error("HTTP " + response.status);
      return response.text().then(function(text) {
        if (isMissingFileResponse(text)) throw new Error("Missing file");
        return text;
      });
    })
    .catch(function() {
      return new Promise(function(resolve, reject) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", url, true);
        xhr.onreadystatechange = function() {
          if (xhr.readyState !== 4) return;
          var ok      = xhr.status >= 200 && xhr.status < 300;
          var hasBody = typeof xhr.responseText === "string" && xhr.responseText.length > 0;
          var local   = xhr.status === 0 && hasBody && !isMissingFileResponse(xhr.responseText);
          if (ok || local) resolve(xhr.responseText);
          else             reject(new Error("HTTP " + xhr.status));
        };
        xhr.onerror = function() { reject(new Error("Network error")); };
        xhr.send();
      });
    });
}

function isMissingFileResponse(text) {
  if (typeof text !== "string") return true;
  return text.indexOf("ERR_FILE_NOT_FOUND") !== -1
      || text.indexOf("404 Not Found")      !== -1;
}

// ─── Title ───────────────────────────────────────────────────────────────────

// Called on iframe load to refresh the center label.
function setTitle() {
  if (!dom.source || !dom.titleSpan) cacheDomElements();
  if (!dom.source || !dom.titleSpan) return;

  try {
    var doc        = dom.source.contentWindow.document;
    var titleEl    = doc.getElementsByTagName("title")[0];
    var pageTitle  = titleEl ? titleEl.innerHTML : "";
    dom.titleSpan.textContent = state.currentIndex + "/" + state.fileCount
      + (pageTitle ? " : " + pageTitle : "");
  } catch (e) {
    dom.titleSpan.textContent = state.currentIndex + "/" + state.fileCount;
  }
}

// ─── Init ────────────────────────────────────────────────────────────────────

async function init() {
  debugLog("init:start", {
    search: window.location.search,
    sankorePresent: !!window.sankore
  });

  cacheDomElements();
  bindFrameHandlers();

  var requestedLanguage = getCurrentLanguage();
  CONFIG.language = await resolveLanguage(requestedLanguage);
  debugLog("language:resolved", {
    requested: requestedLanguage,
    selected: CONFIG.language
  });

  state.theme = resolveTheme();
  applyHostTheme(state.theme);
  bindThemeSync();
  debugLog("theme:resolved", { theme: state.theme });

  await detectAvailableHints();
  debugLog("hints:detected", { fileCount: state.fileCount });

  if (state.fileCount > 0) {
    state.currentIndex = 1;
    loadCurrentHint();
  } else if (dom.source) {
    dom.source.src = buildErrorUrl();
    debugLog("hints:none", { errorSrc: dom.source.src });
  }

  bindNavigationHandlers();
  debugLog("init:done");
}

if (DEBUG) {
  window.addEventListener("error", function(event) {
    debugLog("window:error", {
      message: event.message,
      source: event.filename,
      line: event.lineno,
      column: event.colno
    });
  });

  window.addEventListener("unhandledrejection", function(event) {
    debugLog("window:unhandledrejection", {
      reason: event.reason && event.reason.message ? event.reason.message : String(event.reason)
    });
  });
}

document.addEventListener("DOMContentLoaded", function() {
  init().catch(function(error) {
    console.error("[startupHints] init failed", error);
  });
});