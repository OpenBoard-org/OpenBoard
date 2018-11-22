module.exports = function (grunt) {
    
    var baseSrc = 'js/src';

    grunt.loadNpmTasks('grunt-contrib-watch');
    grunt.loadNpmTasks('grunt-contrib-uglify');

    grunt.initConfig({
        uglify: {
            options: {
                separator: ';'
            },
            compile: {
                src: [
                    baseSrc + '/Sankore/klass.js', 
                    baseSrc + '/**/*.js'
                ],
                dest: 'dist/calculator.js'
            }
        },
        watch: {
            scripts: {
                files: 'js/src/**/*.js',
                tasks: ['scripts:dist']
            }
        }
    });

    grunt.registerTask('default', ['dist', 'watch']);
    grunt.registerTask('dist', ['scripts:dist']);

    grunt.registerTask('scripts:dist', ['uglify:compile']);
};