var gulp = require('gulp');
var uglify = require('gulp-terser');
var htmlmin = require('gulp-htmlmin');
var cssmin = require('gulp-cssmin');
var concat = require('gulp-concat');
var zip = require('gulp-zip');
var fs = require('fs');
var mkdirp = require('mkdirp');
var chalk = require('chalk');
var watch = require('gulp-watch');
var through = require('through2');
// var exec = require('child_process').execSync;
var path = require('path');
var spawn = require('child_process').spawnSync
var removeCode = require('gulp-remove-code');

const readdirSync = (p, a = []) => {
	if (fs.statSync(p).isDirectory())
		fs.readdirSync(p).map(f => readdirSync(a[a.push(path.join(p, f)) - 1], a))
	return a
}

function exec(cmd, args, env) {
	return spawn(cmd, args, {shell: true, stdio: "inherit", env: env || process.env})
}

//Chalk colors
var error = chalk.bold.red;
var success = chalk.green;
var regular = chalk.white;


var isProduction = false
const approximatedSizeReductionOnProductionRemovals = 1570


gulp.task('watch-only', (done) => {
	gulp.watch('./src/js/**/*.js', gulp.series('build-js', 'zip', 'check'));
	gulp.watch('./src/wasm/**/*.cpp', gulp.series('build-cpp', 'zip', 'check'));
	gulp.watch('./src/wasm/**/*.hpp', gulp.series('build-cpp', 'zip', 'check'));
	gulp.watch('./src/html/**/*.html', gulp.series('build-html', 'check'));
	gulp.watch('./src/css/**/*.css', gulp.series('build-css', 'check'));
	gulp.watch('./src/assets/**/*', gulp.series('build-assets', 'check'));
});

gulp.task('init', (done) => {
	//Create our directory structure
	mkdirp('./src', function (err) {
		mkdirp('./src/js', function (err) {
			mkdirp('./src/html', function (err) {
				mkdirp('./src/css', function (err) {
					mkdirp('./src/assets', function (err) {
						done();
					});
				});
			});
		});
	});
});

gulp.task('build-js', (done) => {
	return gulp.src('./src/js/**/*.js')
	.pipe(removeCode({production: isProduction}))
	.pipe(uglify())
	.pipe(gulp.dest('./build/'));
});

gulp.task('build-walt', (done) => {
	return gulp.src('./src/js/**/*.walt')
	.pipe(through.obj(function(file, enc, next) {
		if (file.isNull()) {
			next(null, file);
			return;
		}

		const newFilename = path.basename(file.path, path.extname(file.path)) + '.wasm'
		const newFilePath = __dirname + `/build/${newFilename}`
		exec('node', [`${__dirname}/node_modules/walt-cli/index.js`, file.path, '-o ${newFilePath}'])
		file.path = newFilePath
		file.contents = fs.readFileSync(newFilePath)
		console.log(`${newFilename} is ${file.contents.length} bytes long!`)

		next(null, file);
	}))
});

gulp.task('build-cpp', (done) => {
	const dir = './src/wasm'
	const cppFiles = readdirSync(dir).filter(path => path.endsWith('.cpp'))
	const destPath = 'build/game.wasm'

	const exportedFunctions = ['preinit', 'generateTextures', 'initEngine', 'initGame', 'render']
		.map(fn => `'_${fn}'`)
		.join(",")

	function args(text) {
		return text.split("\n")
	}

	console.log("Compiling: " + cppFiles.join(" "))

	let env = Object.assign({}, process.env)
	if (isProduction) {
		env['EMMAKEN_CFLAGS'] = "-DPRODUCTION"
	}
	if (exec('em++', args(`
		${cppFiles.join(" ")}
		-o ${destPath}
		--std=c++14
		-Os
		-s WASM=1
		-s NO_EXIT_RUNTIME=1
		-s SIDE_MODULE=1
		-s ONLY_MY_CODE=1
		-s "EXPORTED_FUNCTIONS=[${exportedFunctions}]"
		-s TOTAL_MEMORY=16MB
	`), env).status == 0) {
		// exec('wasm2wat', args(`
		// 	${destPath}
		// 	--output=${destPath}.wat
		// `))

		const origSize = fs.statSync(destPath).size

		// optimize .wasm size
		const destPathOpt = `${destPath}-opt`
		exec('wasm-opt', args(`
			-Oz
			${destPath}
			-o ${destPathOpt}`
		))
		const optSize = fs.statSync(destPathOpt).size

		if (optSize < origSize) {
			fs.unlinkSync(destPath)
			console.log(success(`WASM size: ${origSize}, optimized to ${optSize}, diff = ${origSize - optSize}`))
			fs.renameSync(destPathOpt, destPath)
		}
		else {
			fs.unlinkSync(destPathOpt)
			console.log(success(`WASM size: ${origSize}, optimization failed by diff = ${origSize - optSize}`))
		}
	}

	done()
});

gulp.task('build-html', (done) => {
	return gulp.src('./src/html/**/*.html')
		.pipe(removeCode({production: isProduction}))
		.pipe(htmlmin({collapseWhitespace: true}))
		.pipe(gulp.dest('./build/'));
});

gulp.task('build-css', (done) => {
	return gulp.src('./src/css/**/*.css')
		.pipe(cssmin())
		.pipe(gulp.dest('./build/'));
});

gulp.task('build-assets', (done) => {
	return gulp.src('./src/assets/**/*')
		.pipe(gulp.dest('./build/'));
});

gulp.task('zip', (done) => {
	let srcs = ['./build/**/*', '!**/*.wat']

	if (isProduction)
		srcs.push( '!**/auto-reload.js')

	return gulp.src(srcs)
		.pipe(zip('entry.zip')) //gulp-zip performs compression by default
		.pipe(gulp.dest('dist'));
});

gulp.task('check', gulp.series('zip', (done) => {
	var stats = fs.statSync("./dist/entry.zip")
	var fileSize = stats.size;
	const sizeLimit = 13312

	if (fileSize > sizeLimit) {
		console.log(error(`Your zip compressed game is larger than 13kb (${sizeLimit} bytes)!`))
		console.log(regular("Your zip compressed game is " + fileSize + " bytes"));
	} else {
		console.log(success("Your zip compressed game is " + fileSize + " bytes."));
	}

	if (!isProduction) {
		const approximatedSize = fileSize - approximatedSizeReductionOnProductionRemovals
		if (approximatedSize <= sizeLimit) {
			console.log(regular(`gulp build-prod might give you only ~${approximatedSize} bytes!`))
		}
	}

	done();
}));

gulp.task('build', gulp.series('build-html', 'build-cpp', 'build-js', 'build-assets', 'check', (done) => {
	done();
}));

gulp.task('build-prod', gulp.series(cb => { isProduction = true; cb() }, 'build'))
gulp.task('check-prod', gulp.series(cb => { isProduction = true; cb() }, 'check'))

gulp.task('watch', gulp.series('build', 'watch-only', done => done()))