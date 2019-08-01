/*
 
ESP8266 file system builder with PlatformIO support
 
Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
 
// -----------------------------------------------------------------------------
// File system builder
// -----------------------------------------------------------------------------
 
const gulp = require('gulp');
const plumber = require('gulp-plumber');
const htmlmin = require('gulp-htmlmin');
const cleancss = require('gulp-clean-css');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const del = require('del');
const useref = require('gulp-useref');
const gulpif = require('gulp-if');
const inline = require('gulp-inline');
 
/* Clean destination folder */
gulp.task('clean', function() {
    return del(['data/*']);
});
 
/* Copy static files */
gulp.task('files', function() {
    return gulp.src([
            'web/dev/*.{jpg,jpeg,png,ico,gif}',
        ])
        .pipe(gulp.dest('web/build'));
});
 
/* Process HTML, CSS, JS  --- INLINE --- */
gulp.task('inline', function() {
    return gulp.src('web/dev/*.html')
        .pipe(inline({
            base: 'web/dev/',
            js: uglify,
            css: cleancss,
            disabledTypes: ['svg', 'img']
        }))
        .pipe(htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        }))
        .pipe(gzip())
        .pipe(gulp.dest('web/build/'));
})
 
/* Process HTML, CSS, JS */
gulp.task('html', function() {
    return gulp.src('web/dev/*.html')
        .pipe(useref())
        .pipe(plumber())
        .pipe(gulpif('*.css', cleancss()))
        .pipe(gulpif('*.js', uglify()))
        .pipe(gulpif('*.html', htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        })))
        .pipe(gulpif(['*.css','*.js'], gzip()))
        .pipe(gulp.dest('web/build/'));
});
 
/* Build file system */
gulp.task('buildfs', ['clean', 'files', 'html']);
gulp.task('buildfs2', ['clean', 'files', 'inline']);
gulp.task('default', ['buildfs']);
