var toolbarTemplate = 	
			'<h1 class="title">{{fr.njin.i18n.document.title}}</h1>'+
			'<div class="actions">'+
				'<button role="edit"><span>{{fr.njin.i18n.toolbar.edit}}</span></button>'+
				'<button role="view"><span>{{fr.njin.i18n.toolbar.view}}</span></button>'+
				'<button role="reload"><span>{{fr.njin.i18n.toolbar.reload}}</span></button>'+
				'<button role="help"><span>{{fr.njin.i18n.toolbar.help}}</span></button>'+
			'</div>';

var parametersTemplate = 
			'<div class="inline">'+
				'<label>{{fr.njin.i18n.parameters.label.themes}}'+
					'<select name="themes" role="parameter">'+
						'<option value="pad">{{fr.njin.i18n.parameters.label.pad.themes}}</option>'+					
						'<option value="slate">{{fr.njin.i18n.parameters.label.slate.themes}}</option>'+
						'<option value="none">{{fr.njin.i18n.parameters.label.none.themes}}</option>'+
					'</select>'+
				'</label>'+
			'</div>'+
			'<div class="inline">'+
				'<label>{{fr.njin.i18n.memory.parameters.label.cards}}'+
					'<select name="count" role="parameter" data-min="4" data-max="8">'+
					'</select>'+
				'</label>'+
			'</div>'+
			'<div class="inline">'+
				'<label>{{fr.njin.i18n.memory.parameters.label.timeout}}'+
					'<input name="timeout" role="parameter" type="text" min="1" max="60" value="1"/><span> (s)</span>'+
				'</label>'+
			'</div>';
			
			
var cardTemplate = 
			'<div>'+
				'<div class="card-container">'+
					'<div class="cards">'+
						'<div class="front">'+
							"<div class='switch'>"+
								"<label>{{fr.njin.i18n.memory.label.usePicture}}<input type='checkbox' name='switch'></label>"+
							"</div>"+
							"<div class='card'>"+
								"<div class='text'>"+
									"<div>{{frontDisplayValue}}</div>"+
								"</div>"+
								"<div class='picture'>"+
									"<div></div>"+
									"<div class='dropzone'>"+
										"<div>{{fr.njin.i18n.memory.label.drop}}</div>"+
									"</div>"+
								"</div>"+
							"</div>"+
						'</div>'+
						'<div class="back">'+
							"<div class='switch'>"+
								"<label>{{fr.njin.i18n.memory.label.usePicture}}<input type='checkbox' name='switch'></label>"+
							"</div>"+
							"<div class='card'>"+
								"<div class='text'>"+
									"<div>{{backDisplayValue}}</div>"+
								"</div>"+
								"<div class='picture'>"+
									"<div></div>"+
									"<div class='dropzone'>"+
										"<div>{{fr.njin.i18n.memory.label.drop}}</div>"+
									"</div>"+
								"</div>"+
							"</div>"+
						'</div>'+
					'</div>'+
					'<div class="actions">'+
						'<button role="flip">{{fr.njin.i18n.memory.action.flip}}</button>'+
						'<div>{{card.val}}</div>'+
					'</div>'+
				'</div>'+
			'</div>';