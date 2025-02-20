let     { exec }    = require('child_process');
const { promisify } = require('util');
const {readdir,writeFile}            = require('fs/promises')
const data   = require('./audio_data.json');

exec = promisify(exec);



const regexPatterns = {
    elapsedTime: /([\d.]+)\s+seconds time elapsed/,
    userTime: /([\d.]+)\s+seconds user/,
    sysTime: /([\d.]+)\s+seconds sys/,
    cpuUtilized: /([\d.]+)\s+CPUs utilized/,
    taskClock: /([\d.]+)\s+msec task-clock:u/,
    pageFaults: /([\d,]+)\s+page-faults:u/,
    instructions: /([\d,]+)\s+instructions:u/,
    cycles: /([\d,]+)\s+cycles:u/,
    stalledCyclesFrontend: /([\d,]+)\s+stalled-cycles-frontend:u/,
    branches: /([\d,]+)\s+branches:u/,
    branchMisses: /([\d,]+)\s+branch-misses:u/,
};

function parsePerfOutput(output) {
    let results = {};

    for (let key in regexPatterns) {
        let match = output.match(regexPatterns[key]);
        if (match) {
            let value = match[1].replace(/,/g, '');
            results[key] = parseFloat(value);
        } else 
            results[key] = null;
        
    }

    results["branch_miss_%"] = (results["branchMisses"]*100/results["branches"])

    return results;
}

async function run(cmd) {
    try {
        const { stdout, stderr } = await exec(cmd);
        if (stderr) {
            console.error('Error:', stderr);
        }
        return parsePerfOutput(stderr);
    } catch (error) {
        console.error('Execution failed:', error);
    }
}


async function benchmark(file) {
    let out = await Promise.all([
        await run(`perf stat ffmpeg -i "${file}" -f segment -segment_time 1 -c copy out1/a_%03d.wav`),
        await run(`perf stat ./conv "${file}" AUTO "1" "./out2/"`)
    ])

    return {
        audio:file,
        duration:data[file]["duration"],
        ffmpeg: out[0],
        conv: out[1]
    }
}

async function main(inp) {
    let files = await readdir(inp)
    files     = files.map(n=>`${inp}/${n}`)

    let out = []

    for (let file of files) {
        let p = await benchmark(file)
        out.push(p)
        console.log(`Benchmarked ${file}`)
    }

    console.log(out)
    await writeFile("benchmark.json",JSON.stringify(out,null,2),{encoding:"utf8"})
}

main("./Blue Jay");
