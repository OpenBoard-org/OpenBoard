(function() {
    Sankore.Util.I18N.catalogs.set('fr', {
        layout: {
            classic_name: 'Calculatrice standard',
            new_name: 'Nouvelle calculatrice'
        },
        text: {
            del: 'EFF',
            comma: ','
        },
        command: {
            zero: 'Chiffre 0',
            one: 'Chiffre 1',
            two: 'Chiffre 2',
            three: 'Chiffre 3',
            four: 'Chiffre 4',
            five: 'Chiffre 5',
            six: 'Chiffre 6',
            seven: 'Chiffre 7',
            eight: 'Chiffre 8',
            nine: 'Chiffre 9',
            plus: 'Addition',
            minus: 'Soustraction',
            times: 'Multiplication',
            divide: 'Division',
            euclidean_divide: 'Division euclidienne',
            equal: 'Egalité',
            comma: 'Virgule',
            open_parenthesis: 'Parenthèse ouvrante',
            close_parenthesis: 'Parenthèse fermante',
            op: 'Opérateur constant',
            memory_add: 'Addition mémoire',
            memory_sub: 'Soustraction mémoire',
            memory_recall: 'Rappel mémoire',
            memory_clear: 'R.A.Z. mémoire',
            clear: 'R.A.Z.',
            left: 'Déplacement à gauche',
            right: 'Déplacement à droite',
            del: 'Suppression'
        },
        controls: {
            editor: 'Editeur',
            reset: 'RAZ'
        },
        editor: {
            run_button: 'Utiliser',
            remove_alert: 'Supprimer cette calculatrice ?',
            layout_name: {
                label: 'Nom'
            },
            layout_description: {
                label: 'Description'
            },
            assignation: {
                enabled: 'Cliquez sur une touche pour la modifier',
                disabled: 'Cette calculatrice n\'est pas modifiable',
                text: {
                    label: 'Texte à afficher'
                },
                command: {
                    label: 'Commande'
                },
                use_limit: {
                    label: 'Nb d\'utilisation',
                    help: '0 pour désactiver, vide pour illimité'
                }
            }
        },
        error: {
            common: 'Erreur',
            zero_division: 'Erreur div/0'
        }
    });
})();