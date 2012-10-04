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
			'</div>';

var objectTemplate = 
			'<div id="{{object}}" class="object weight">'+
				'<div class="tools">'+
					'<button role="remove">{{fr.njin.i18n.balance.parameters.label.remove}}</button>'+
				'</div>'+
				'<div class="amount"><input type="text" name="weight"></div><div class="bg"></div>'+
			'</div>';
			
var weightTemplate = 
			'<div id="{{object}}" class="weight right w{{weight}}" data-weight="{{weight}}"><div class="amount">{{weight}}</div><div class="bg"></div></div>';						