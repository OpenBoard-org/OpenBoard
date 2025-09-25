 var current = 1, files_count = 0;
    var lang = "fr"; // langue forcée
    var tutorialUrl = "tutoriel/index.html"; // cible du bouton tutoriel

    async function init(){
      var stop = false;

      // Charger tous les fichiers disponibles (1.html, 2.html, etc.)
      while (1) {
        try {
          await makeRequest('locales/' + lang + '/' + (files_count + 1) + '.html');
          files_count++;
        } catch (error) {
          stop = true;
        }
        if (stop) break;
      }

      if (files_count > 0) {
        // Charger la première page
        $("#source").attr("src", "locales/" + lang + "/1.html");

        $("#right").click(function () {
          if (current < files_count) {
            current++;
          } else {
            current = 1;
          }
          $("#source").attr("src", "locales/" + lang + "/" + current + ".html");
        });

        $("#left").click(function () {
          if (current > 1) {
            current--;
          } else {
            current = files_count;
          }
          $("#source").attr("src", "locales/" + lang + "/" + current + ".html");
        });
      } else {
        $("#source").attr("src", "locales/" + lang + "/error.html");
      }

      // Bouton Accès au tutoriel (ouvre dans un nouvel onglet)
      $("#btn-tuto").on("click", function(e){
        e.preventDefault();
        window.open(tutorialUrl, "_blank", "noopener,noreferrer");
      });

      // Navigation clavier (flèches gauche/droite)
      document.addEventListener("keydown", function(e){
        if (e.key === "ArrowRight") $("#right").click();
        if (e.key === "ArrowLeft")  $("#left").click();
      });
    }

    // Ajax GET (au lieu de POST) pour fichiers statiques
    function makeRequest(url) {
      return new Promise(function(resolve, reject) {
        $.ajax({
          url: url,
          type: 'GET',
          dataType: 'html',
          cache: false,
          success: function(data) { resolve(data); },
          error: function(xhr, status, err) { reject(err || status); }
        });
      });
    }

    // Mettre à jour le titre central avec le <title> de la page chargée
    function setTitle(){
      var iframe = document.getElementById("source");
      try {
        var doc = iframe.contentWindow.document;
        var titleEls = doc.getElementsByTagName("title");
        var titleTxt = (titleEls.length ? titleEls[0].innerHTML : "");
        $("#title span").text(current + "/" + files_count + (titleTxt ? " : " + titleTxt : ""));
      } catch(e){
        $("#title span").text(current + "/" + files_count);
      }
    }

    $(document).ready(function(){
      init();
    });