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
				'<label>{{fr.njin.i18n.tableur.parameters.label.operator}}'+
					'<select name="operator" role="parameter">'+
						'<option value="*">*</option>'+
						'<option value="+">+</option>'+						
						'<option value="-">-</option>'+						
						'<option value="/">/</option>'+						
					'</select>'+
				'</label>'+
			'</div>'+
			'<div class="inline">'+
				'<label>{{fr.njin.i18n.tableur.parameters.label.row}}'+ 
					'<select name="row" role="parameter" data-min="1" data-max="12">'+
					'</select>'+
				'</label>'+
			'</div>'+
			'<div class="inline">'+
				'<label>{{fr.njin.i18n.tableur.parameters.label.col}}'+ 
					'<select name="col" role="parameter" data-min="1" data-max="12">'+
					'</select>'+
				'</label>'+
			'</div>'+
			'<div style="display:none;">'+
				'<label>{{fr.njin.i18n.tableur.parameters.label.reste}}'+ 
					'<input type="checkbox" name="reste" role="parameter">'+
				'</label>'+
			'</div>';