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

 var txt_centr=	"<h2>Centrioles - Tug of chromosomes</h2>"+
				"<h4>Structure:</h4><span><p>Nine triplets of microtubules surrounded by a some number of proteins.</p></span>"+
				"<h4>Role:</h4><span><p>Separate the different chromosomes during cell division.</p></span>"+
				"<h4>Operation:</h4><span><p>Centrioles, once placed at the two poles of the cell, deploy the microtubules to the centromeres of chromosomes and pull them to them to separate them.</p></span>"
				
 var txt_rib =	"<h2>Ribosome - Decoders</h2>"+
				"<h4>Structure:</h4><span><p>Formed by two subunits composed of ribosomal RNA and proteins.</p></span>"+
				"<h4>Role:</h4><span><p>The ribosome synthesizes proteins.</p></span>"+
				"<h4>Operation:</h4><span><p>A strand of mRNA (messenger) pass through the ribosome and tRNA (transfer) between the large subunit where the codon (group of three nitrogen bases) is one who is in front of the mRNA. This tRNA carries with it a specific amino acid that is added а chain already assembled.</p></span>"

 var txt_arn =	"<h2>RNA - Multifunction</h2>"+
				"<h4>Structure:</h4><span><p>Very similar DNA with the difference it has only one strand and thymine (T) of DNA is replaced by uracil (U). In addition, it is chemically more stable than DNA, so it is not used for information storage, just if in long term.</p></span>"+
				"<h4>Role:</h4><span><p>Multiple, there are RNA transport, passenger, cruise, guides, satellite ...</p></span>"+
				"<h4>Operation:</h4><span><p>Copying of genetic information, the opening of the double helix of DNA, and then copying the RNA codons. It can then exit the nucleus, the DNA can not do it.</p></span>"

 var txt_nucl =	"<h2>Nucleolus - Factory RNA</h2>"+
				"<h4>Structure:</h4><span><p>Composed of any membrane, a cluster of proteins and RNA.</p></span>"+
				"<h4>Role:</h4><span><p>Location of the RNA transcripts, including RNA (ribosomal), which combines with protein, will form the two subunits of ribosomes.</p></span>"+
				"<h4>Operation:</h4><span><p>Creation of a ribosome: Transcription of rRNA and protein needed (this step is performed in the cytoplasm by other ribosomes) that fall within the nucleus, association of molecules frankly formed a new ribosome, which leaves the nucleus to play its role.</p></span>"

	