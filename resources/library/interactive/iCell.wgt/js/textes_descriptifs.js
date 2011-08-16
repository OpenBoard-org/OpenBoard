/* Pour info, les span йtaient une tentative de mise en forme via CSS mais j'ai trouvй plus pratique, et je les ai laissйs au cas oщ... */
 
 var txt_vesicule = 	"<h2>Vesicle<br/>Transporter</h2>"+
						"<h4>Structure:</h4><span><p>Phospholipid bilayer</p></span>"+
						"<h4>Role:</h4><span><p>Transport of proteins and other components inside the cell to the exterior (exocytosis) or inward (endocytosis).</p></span>"+
						'<h4>Operation:</h4><span><p>Transport "towed" by Protein building on the cytoskeleton.</p></span>'
						
 var txt_lysosome = "<h2>Lysosome<br/>A stomach cell.</h2>"+
					"<h4>Structure:</h4><span><p>Phospholipid bilayer</p></span>"+
					"<h4>Role:</h4><span><p>А intracellular digestion with enzymes.</p></span>"+
					"<h4>Operation:</h4><span><p>Absorbes nutrient uptake or damaged cellular components by endocytosis, digest them and then distributes the results of the chemical reaction in the cell and finally expels of waste by exocytosis.</p></span>"
 
 var txt_mitoch = 	"<h2>Mitochondrie<br/>Batteries</h2>"+
					"<h4>Structure:</h4><span><p>Two phospholipid bilayers called mitochondrial membranes, one external and one internal. The mitochondria contain ribosomes, ATP of DNA and other molecules.</p></span>"+
					"<h4>Role:</h4><span><p>Powerhouse of the cell.</p></span>"+
					"<h4>Operation:</h4><span><p>Energy - in the form of ATP (adenosine triphosphate) - comes from various stages of chemical reactions starting from a glucose molecule.</p></span>"
 
 var txt_golgi =	"<h2>Apparatus golgi<br/>Miniature plants</h2>"+
					"<h4>Structure:</h4><span><p>Has a stack of flattened membrane saccules.</p></span>"+
					"<h4>Role:</h4><span><p>Modification of proteins during a journey through its saccules.</p></span>"+
					"<h4>Operation:</h4><span><p>Chemical reactions, including glycosylation.</p></span>"
 
 var txt_rer =	"<h2>Rough endoplasmic reticulum - Tunnel</h2>"+
				"<h4>Structure:</h4><span><p>Consists of a phospholipid bilayer studded with ribosomes (an aspect <i> rough </ i>) define the light, an internal space that can be compared to a tunnel.</p></span>"+
				"<h4>Role:</h4><span><p>More specialized than the real, he participated in the transport and а finalizing the proteins that are synthesized by ribosomes.</p></span>"+
				'<h4>Operation:</h4><span><p>Proteins "fall" in the light of the RER where they are modified and displaced. They leave the RER in a vesicle membrane after it.</p></span>'
 
 var txt_noy = 	"<h2>Core - The safe</h2>"+
				"<h4>Structure:</h4><span><p>Surrounded by a double membrane, that called the nuclear envelope, in places linked with the RER. These two membranes protect regular intervals formed nuclear pores. It located within the nucleolus and the DNA in the form of chromatin or chromosomes.</p></span>"+
				"<h4>Role:</h4><span><p>Storing all the genetic information necessary for a life of the cell.</p></span>"+
				"<h4>Operation:</h4><span><p>Copying of genetic information on mRNA.</p></span>"
 
 var txt_rel = 	"<h2>Smooth endoplasmic reticulum - REL</h2>"+
				"<h4>Structure:</h4><span><p>Like that of the RER, with the difference that the membrane is studded with ribosomes, as its smooth <i>appearance</i>.</p></span>"+
				"<h4>Role:</h4><span><p>Phospholipid synthesis, calcium storage, transformation of certain molecules external (drugs, alcohol, ...). In some cells, the LRA also performs additional functions, such as the production of hormones, gastric acid, etc.</p></span>"+
				"<h4>Operation:</h4><span><p>It is the seat of many complex chemical reactions (eg detoxification, various syntheses).</p></span>"
 
 var txt_adn =	"<h2>DNA - The Book of Life</h2>"+
				"<h4>Structure:</h4><span><p>A scale in form of the famous double helix consists of two columns sugar-phosphate-sugar-phosphate ... and whose levels are called nitrogenous bases.</p></span>"+
				"<h4>Role:</h4><span><p>DNA contains all the information needed for a life.</p></span>"+
				"<h4>Operation:</h4><span><p>All information is written using the four letters A, T, G and C. Using these combinations, it is possible to write anything useful about cell.</p></span>"

 var txt_centr=	"<h2>Centrioles - Remorqueurs de choromosomes</h2>"+
				"<h4>Structure:</h4><span><p>Neuf triplets de microtubules entourés par un certain nombre de protéines.</p></span>"+
				"<h4>Role:</h4><span><p>Séparer les différents chromosomes durant la division cellulaire.</p></span>"+
				"<h4>Operation:</h4><span><p>Les centrioles, une fois placés aux deux poles de la cellule, déploient des microtubules vers les centromères des chromosomes et les tirent vers eux pour les séparer.</p></span>"
				
 var txt_rib =	"<h2>Ribosome - Décodeurs</h2>"+
				"<h4>Structure:</h4><span><p>Formé par deux sous-unités composées d'ARN ribosomique et de protéines.</p></span>"+
				"<h4>Role:</h4><span><p>Le ribosome synthétise les protéines.</p></span>"+
				"<h4>Operation:</h4><span><p>Un brin d'ARNm (messager) passe dans le ribosome et un ARNt (de transfert) entre dans la grande sous-unité si son codon (groupe de trois bases azotées) correspond а celui qui est en face sur l'ARNm. Cet ARNt porte avec lui un acide aminé spécifique qui est ajouté а la chaine déja assemblée.</p></span>"

 var txt_arn =	"<h2>ARN - Multifonction</h2>"+
				"<h4>Structure:</h4><span><p>Très similaire а l'ADN а la différence qu'il ne possède qu'un brin et que la thymine (T) de l'ADN est remplacée par l'uracile (U). De plus, il est chimiquement plus instable que l'ADN, c'est pourquoi il n'est pas utilisé pour le stockage d'informations а long terme.</p></span>"+
				"<h4>Role:</h4><span><p>Multiples, il existe des ARN de transport, messagers, régulateus, guides, satellites, ...</p></span>"+
				"<h4>Operation:</h4><span><p>La copie d'informations génétiques se fait grвce а l'ouverture de la double-hélice d'ADN, puis la copie des codons sur l'ARN. Celui-ci peut alors sortir du noyau, ce que l'ADN ne peut pas faire.</p></span>"

 var txt_nucl =	"<h2>Nucléole - Fabrique d'ARN</h2>"+
				"<h4>Structure:</h4><span><p>Composé d'aucune membranne, c'est un agglomérat de protéines et d'ARN.</p></span>"+
				"<h4>Role:</h4><span><p>Lieu de la transcription d'ARN, nottament d'ARNr (ribosomiques) qui, associés avec des protéines, vont former les deux sous-unités des ribosomes.</p></span>"+
				"<h4>Operation:</h4><span><p>Création d'un ribosome: Transcription des ARNr ainsi que des protéines nécessaires (cette étape est effectuée dans le cytoplasme par d'autres ribosomes) qui rentrent dans le noyau, association des molécules fraоchement formées en un nouveau ribosome, qui sort du noyau pour jouer son role.</p></span>"

	