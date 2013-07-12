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
				'<div>{{fr.njin.i18n.parameters.label.themes}}</div>'+
					'<select name="themes" role="parameter">'+
						'<option value="pad">{{fr.njin.i18n.parameters.label.pad.themes}}</option>'+					
						'<option value="slate">{{fr.njin.i18n.parameters.label.slate.themes}}</option>'+
						'<option value="none">{{fr.njin.i18n.parameters.label.none.themes}}</option>'+
					'</select>'+
			'</div>'+
			'<div class="inline">'+
				'<div>{{fr.njin.i18n.transformation.parameters.label.number}}</div>'+
					'<input  class="tiny" type="number" name="number" min="2" role="parameter">'+				
			'</div>'+
			'<div id="edit-cards">'+
			'</div>';