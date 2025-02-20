let     { exec }    = require('child_process');
const { promisify } = require('util');
const {readdir,writeFile}            = require('fs/promises')


exec = promisify(exec);

async function run(file){
    let {stdout} = await exec(`ffprobe -v quiet -print_format json -show_format  "${file}"`)
    stdout = JSON.parse(stdout)
    return stdout.format
}

async function main(inp) {
    let files = await readdir(inp)
    files     = files.map(n=>`${inp}/${n}`)

    let out = await Promise.all(files.map(async(file)=>{
        const data = await run(file)
        return `"${file}":${JSON.stringify(data)}`
    }))

    out = JSON.parse(`{${out.join(',\n')}}`)

    console.log(out)

    await writeFile("audio_data.json",JSON.stringify(out,null,2),{encoding:"utf8"})
}

main("./Blue Jay");
