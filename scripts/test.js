const process = require("node:process");
const os = require("node:os");
const { spawn, spawnSync } = require("node:child_process");
const sysPath = require("node:path");
const waterfall = require("async/waterfall");
const { explore } = require("fs-explorer");
const { findFile } = require("../generator/FileUtils");

const platform = os.platform() === "win32" ? (/cygwin/.test(process.env.HOME) ? "Cygwin" : "x64") : "*-GCC";
const exeSuffix = os.platform() === "win32" ? ".exe" : "";

const config = {
    Debug: {
        exe: findFile(`out/install/${ platform }-Debug/bin/luajit${ exeSuffix }`, __dirname),
        env: {},
    },
    Release: {
        exe: findFile(`out/install/${ platform }-Release/bin/luajit${ exeSuffix }`, __dirname),
        env: {},
    },
};

if (os.platform() !== "win32") {
    config.Debug.env = {
        LUA_CPATH: `${ sysPath.resolve(config.Debug.exe, "../../lib/lua/?.so") };${ spawnSync(config.Debug.exe, ["-e", "print(package.cpath)"]).stdout.trim() }`
    };

    config.Release.env = {
        LUA_CPATH: `${ sysPath.resolve(config.Release.exe, "../../lib/lua/?.so") };${ spawnSync(config.Release.exe, ["-e", "print(package.cpath)"]).stdout.trim() }`
    };
}

const run = (file, cwd, env, next) => {
    const { BUILD_TYPE, OPENCV_BUILD_TYPE } = process.env;
    if (BUILD_TYPE && BUILD_TYPE !== env.BUILD_TYPE || OPENCV_BUILD_TYPE && OPENCV_BUILD_TYPE !== env.OPENCV_BUILD_TYPE) {
        next(0, null);
        return;
    }

    console.log("\nRunning", file, env);
    const extname = sysPath.extname(file);

    const args = [];

    if (extname === ".lua") {
        args.push(config[env.BUILD_TYPE].exe, [file]);
    } else {
        throw new Error(`Unsupported extenstion ${ extname }`);
    }

    console.log(args.flat().map(arg => (arg.includes(" ") ? `"${ arg }"` : arg)).join(" "));

    args.push({
        stdio: "inherit",
        env: Object.assign({}, config[env.BUILD_TYPE].env, process.env, env),
        cwd,
    });

    const child = spawn(...args);

    child.on("error", next);
    child.on("close", next);
};

const options = {
    cwd: sysPath.resolve(__dirname, ".."),
    includes: [],
    "--": false,
};

for (const arg of process.argv.slice(2)) {
    if (!options["--"] && arg === "--") {
        options["--"] = true;
    } else if (options["--"]) {
        options.includes.push(arg);
    } else if (arg === "--Debug" || arg === "--Release") {
        process.env.BUILD_TYPE = arg.slice(2);
    } else {
        options.cwd = sysPath.resolve(arg);
        options["--"] = true;
    }
}

const INCLUDED_EXT = [".lua"];
const EXCLUDED_FILES = ["init.lua"];
const { cwd, includes } = options;

explore(sysPath.join(cwd, "samples"), (path, stats, next) => {
    const file = sysPath.relative(cwd, path);
    const basename = sysPath.basename(file);
    const extname = sysPath.extname(file);

    if (
        !INCLUDED_EXT.includes(extname) ||
        EXCLUDED_FILES.includes(basename) ||
        includes.length !== 0 && !includes.some(include => basename.startsWith(include))
    ) {
        next();
        return;
    }

    waterfall([
        next => {
            run(file, cwd, {
                BUILD_TYPE: "Release",
                OPENCV_BUILD_TYPE: "Release",
            }, next);
        },

        (signal, next) => {
            run(file, cwd, {
                BUILD_TYPE: "Debug",
                OPENCV_BUILD_TYPE: "Debug",
            }, next);
        },
    ], (code, signal) => {
        next(code);
    });
}, (path, stats, files, state, next) => {
    const basename = sysPath.basename(path);
    const skip = state === "begin" && (basename[0] === "." || basename === "BackUp");
    next(null, skip);
}, err => {
    if (err) {
        console.log(err);
        // const code = err.flat(Infinity)[0];
        // process.exitCode = code;
    }
});