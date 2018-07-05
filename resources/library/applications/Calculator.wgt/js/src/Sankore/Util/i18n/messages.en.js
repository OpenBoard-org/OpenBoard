(function() {
    Sankore.Util.I18N.catalogs.set('en', {
        layout: {
            classic_name: 'Basic calculator',
            new_name: 'New calculator'
        },
        text: {
            del: 'DEL',
            comma: '.'
        },
        command: {
            zero: '0 digit',
            one: '1 digit',
            two: '2 digit',
            three: '3 digit',
            four: '4 digit',
            five: '5 digit',
            six: '6 digit',
            seven: '7 digit',
            eight: '8 digit',
            nine: '9 digit',
            plus: 'Addition',
            minus: 'Subtraction',
            times: 'Multiplication',
            divide: 'Division',
            euclidean_divide: 'Euclidean division',
            equal: 'Equal',
            comma: 'Comma',
            open_parenthesis: 'Open parenthesis',
            close_parenthesis: 'Close parenthesis',
            op: 'Constant operator',
            memory_add: 'Memory addition',
            memory_sub: 'Memory substraction',
            memory_recall: 'Memory recall',
            memory_clear: 'Memory clear',
            clear: 'Clear',
            left: 'Move left',
            right: 'Move right',
            del: 'Delete'
        },
        controls: {
            editor: 'Editor',
            reset: 'RST'
        },
        editor: {
            run_button: 'Run',
            remove_alert: 'Delete this calculator?',
            layout_name: {
                label: 'Name'
            },
            layout_description: {
                label: 'Description'
            },
            assignation: {
                enabled: 'Click on a button to edit it',
                disabled: 'This calculator is not editable',
                text: {
                    label: 'Display text'
                },
                command: {
                    label: 'Command'
                },
                use_limit: {
                    label: 'Use limit',
                    help: '0 for disabled, empty for unlimited'
                }
            }
        },
        error: {
            common: 'Error',
            zero_division: 'Div/0 error'
        }
    });
})();