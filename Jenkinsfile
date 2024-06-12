libraryPipeline(component:'voip-patrol',
                dockerfile: 'docker/Dockerfile.libon',
                releaseSlackChannel: '#team-voice',
                githubRepo: 'voip_patrol',
                pushPRImages: true
) {
   sh 'git submodule update --init'
}
