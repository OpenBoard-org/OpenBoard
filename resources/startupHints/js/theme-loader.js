// Inject theme stylesheet from URL param (?theme=dark|light)
(function () {
  var params = new URLSearchParams(window.location.search);
  var theme = params.get("theme");
  if (theme !== "dark" && theme !== "light") {
    theme = "light";
  }

  var themeStylesheet = document.createElement("link");
  themeStylesheet.rel = "stylesheet";
  themeStylesheet.type = "text/css";
  themeStylesheet.href = "../../css/" + theme + ".css";
  document.head.appendChild(themeStylesheet);
})();