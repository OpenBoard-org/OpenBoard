"use strict";

// ─── Config ──────────────────────────────────────────────────────────────────

var CONFIG = {
  language:    "fr",
  tutorialUrl: "tutoriel/index.html",
  localesRoot: "locales",
  maxHints:    50
};

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
  titleSpan:      null,
  tutorialButton: null
};

function cacheDomElements() {
  dom.source         = document.getElementById("source");
  dom.left           = document.getElementById("left");
  dom.right          = document.getElementById("right");
  dom.titleSpan      = document.querySelector("#title span");
  dom.tutorialButton = document.getElementById("btn-tuto");
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

function getCurrentTheme() {
  return detectThemeFromUrl() || detectThemeFromSankore() || state.theme || "light";
}

// ─── Theme application ───────────────────────────────────────────────────────

// Applies data-theme attribute to the host document (drives nav colors).
function applyHostTheme(theme) {
  var safeTheme = normalizeTheme(theme) || "light";
  document.documentElement.setAttribute("data-theme", safeTheme);
  applyThemeToDocument(document, safeTheme);
}

// Injects or updates the theme stylesheet inside an iframe document.
function applyThemeToDocument(doc, theme) {
  if (!doc || !doc.head) return;

  var safeTheme = normalizeTheme(theme) || "light";
  var themeUrl  = new URL("css/" + safeTheme + ".css", window.location.href).href;
  var linkId    = "ob-theme-css";
  var link      = doc.getElementById(linkId);

  if (!link) {
    link      = doc.createElement("link");
    link.id   = linkId;
    link.rel  = "stylesheet";
    link.type = "text/css";
    doc.head.appendChild(link);
  }

  if (link.href !== themeUrl) link.href = themeUrl;
}

function applyThemeToCurrentHint() {
  try {
    var doc = dom.source && dom.source.contentWindow && dom.source.contentWindow.document;
    if (doc) applyThemeToDocument(doc, state.theme);
  } catch (e) {
    // Cross-origin guard — safe to ignore.
  }
}

// Connects to sankore.themeChanged and retries until the channel is ready.
function bindThemeSync() {
  function tryConnect() {
    if (!window.sankore
      || !window.sankore.themeChanged
      || typeof window.sankore.themeChanged.connect !== "function") {
      return false;
    }

    window.sankore.themeChanged.connect(function(isDark) {
      state.theme = isDark ? "dark" : "light";
      applyHostTheme(state.theme);
      applyThemeToCurrentHint();
    });

    var current = detectThemeFromSankore();
    if (current && current !== state.theme) {
      state.theme = current;
      applyHostTheme(state.theme);
      applyThemeToCurrentHint();
    }

    return true;
  }

  if (tryConnect()) return;

  var attempts = 0;
  var timer = setInterval(function() {
    if (tryConnect() || ++attempts >= 40) clearInterval(timer);
  }, 100);
}

// ─── Navigation ──────────────────────────────────────────────────────────────

function loadCurrentHint() {
  if (!dom.source) return;
  state.theme      = getCurrentTheme();
  applyHostTheme(state.theme);
  dom.source.src   = buildHintUrl(state.currentIndex);
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
  dom.source.addEventListener("load", applyThemeToCurrentHint);
}

// ─── Tutorial (COMING SOON) ───────────────────────────────────────────────────
/*
function bindTutorialHandler() {
  if (!dom.tutorialButton) return;
  dom.tutorialButton.addEventListener("click", function(e) {
    e.preventDefault();
    window.open(CONFIG.tutorialUrl, "_blank", "noopener,noreferrer");
  });
}
*/


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

window.setTitle = setTitle;

// ─── Init ────────────────────────────────────────────────────────────────────

async function init() {
  cacheDomElements();
  bindFrameHandlers();

  state.theme = getCurrentTheme();
  applyHostTheme(state.theme);
  bindThemeSync();

  await detectAvailableHints();

  if (state.fileCount > 0) {
    state.currentIndex = 1;
    loadCurrentHint();
  } else if (dom.source) {
    dom.source.src = buildErrorUrl();
  }

  bindNavigationHandlers();
  bindTutorialHandler();
}

document.addEventListener("DOMContentLoaded", init);