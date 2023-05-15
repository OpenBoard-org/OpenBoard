// translate tooltips
const de = [
  "Links drehen",
  "Rechts drehen",
  "Kamera wählen",
  "Bildgröße wählen",
  "Bild aufnehmen"
];

const fr = [
  "Pivoter à gauche",
  "Pivoter à droite",
  "Sélectionner la caméra",
  "Taille de la capture",
  "Capturer"
];

function translate() {
  const buttons = [
    document.getElementById("rotleft"),
    document.getElementById("rotright"),
    document.getElementById("selectCamera"),
    document.getElementById("selectSize"),
    document.getElementById("snapshot")
  ];

  let lang = sankore.lang.substr(0, 2);
  let translation;

  switch (lang) {
    case "de":
      translation = de;
      break;
    case "fr":
      translation = fr;
      break;
    default:
      return;
  }

  for (ix = 0; ix < buttons.length; ++ix) {
    // get the span below the button
    let span = buttons[ix].getElementsByTagName("span");

    if (span) {
      span[0].textContent = translation[ix];
    }
  }
}
