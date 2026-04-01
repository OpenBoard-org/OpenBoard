// Inject theme stylesheet from URL param (?theme=dark|light)
// and apply critical variables immediately to avoid first-paint color flicker.
(function () {
  var params = new URLSearchParams(window.location.search);
  var theme = params.get("theme");
  if (theme !== "dark" && theme !== "light") {
    theme = "light";
  }

  document.documentElement.setAttribute("data-theme", theme);

  var criticalVars = theme === "dark"
    ? {
        bg: "#1f2329",
        text: "#e5e7eb",
        brand: "#8ba3d7",
        line: "#4b5563",
        hostContentBg: "#1b2027",
        hostFrameBg: "#1f2329"
      }
    : {
        bg: "#ffffff",
        text: "#1f2937",
        brand: "#6682b5",
        line: "#e5e7eb",
        hostContentBg: "#fafafa",
        hostFrameBg: "#ffffff"
      };

  var criticalStyle = document.createElement("style");
  criticalStyle.id = "theme-critical-vars";
  criticalStyle.textContent =
    ":root,:root[data-theme=\"" + theme + "\"]{" +
    "--bg:" + criticalVars.bg + ";" +
    "--text:" + criticalVars.text + ";" +
    "--brand:" + criticalVars.brand + ";" +
    "--line:" + criticalVars.line + ";" +
    "--host-content-bg:" + criticalVars.hostContentBg + ";" +
    "--host-frame-bg:" + criticalVars.hostFrameBg + ";" +
    "}";
  document.head.appendChild(criticalStyle);

  var themeStylesheet = document.createElement("link");
  themeStylesheet.rel = "stylesheet";
  themeStylesheet.type = "text/css";
  themeStylesheet.href = "../../css/" + theme + ".css";
  themeStylesheet.onload = function () {
    if (criticalStyle.parentNode) {
      criticalStyle.parentNode.removeChild(criticalStyle);
    }
  };
  document.head.appendChild(themeStylesheet);
})();