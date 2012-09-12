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
						
			
var cellTemplate = 
			'<div data-i={{cell.i}} data-j={{cell.j}}>'+
				'<div class="debug">'+
					'<span>{{cell.i}},{{cell.j}}</span>'+
				'</div>'+
				'<div>'+
					'<span class="operation">{{cell.operation}}</span>'+
					'<span class="equal">=</span>'+
					'<span class="result" data-operation="{{cell.operation}}"></span>'+
					'<div class="actions">'+
						'<button role="submit">{{fr.njin.i18n.morpion.cell.submit}}</button>'+
					'</div>'+
				'</div>'+
			'</div>';