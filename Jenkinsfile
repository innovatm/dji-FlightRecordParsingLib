pipeline {
    agent {
        label {
        label ""
        customWorkspace "workspace/"+"dji-flightlog-decoder-${BRANCH_NAME}-${BUILD_ID}".replaceAll("/","-")
        }
    }
    options {
        buildDiscarder(logRotator(numToKeepStr: '20'))
    }
    environment {
        HARBOR_REGISTRY = "harbor.innov-atm.com"
        SLACK_CHANNEL = "#system"
        DOCKER_REPO = "dji-flighlog-decoder"
    }
    tools {
        jdk 'jdk17'
    }
    stages {
        stage('init') {
            when {
                anyOf {
                    changeRequest()
                    branch 'develop'
                    environment name : 'TAG_NAME', value : 'dev'
                    environment name : 'TAG_NAME', value : 'prod'
                    environment name : 'TAG_NAME', value : 'staging'
                }
            }
            steps {
                script {
                    env.VERSION= 'dev';
                    if ( env.TAG_NAME == 'dev' ) {
                        env.DOCKER_TAG = 'dev';
                        env.DOCKER_RELEASE_TAG = 'dev';
                        env.DEPLOY_ENV= 'dev';
                        env.DEPLOY_SERVER = "62.4.14.218";
                        env.DEPLOY_FQDN="dji-fld.dev.dronekeeper.com";
                    } else if ( env.TAG_NAME == 'staging' ) {
                        env.DOCKER_TAG = 'staging';
                        env.DOCKER_RELEASE_TAG = 'staging';
                        env.DEPLOY_ENV= 'staging';
                        env.DEPLOY_SERVER = "51.158.20.112";
                        env.DEPLOY_FQDN="dji-fld.staging.dronekeeper.com";
                    } else if ( env.TAG_NAME == 'prod' ) {
                        env.DOCKER_TAG = 'prod';
                        env.DOCKER_RELEASE_TAG=sh(returnStdout: true, script: '''
                            git tag -l --points-at HEAD| grep -v -E 'dev|prod' | xargs -n2
                        ''');
                        env.DEPLOY_ENV= 'prod';
                        env.DEPLOY_SERVER = "62.210.28.140";
                        env.DEPLOY_FQDN="dji-fld.app.dronekeeper.com";
                    }
                    sh '''
                        if [ "$DOCKER_TAG" = "prod" ] && [ ! -n "$DOCKER_RELEASE_TAG" ]; then
                            echo "Prod (prod) tag found without release tag"
                            echo "You need prod and release tag on same commit to deploy!"
                            exit 1
                        fi
                    '''
                }
            }
        }
        stage('release') {
            when { not { environment name: 'DOCKER_TAG', value: '' } }
            stages() {
                stage('notify') {
                    when { environment name: 'TAG_NAME', value: 'prod' }
                    steps {
                        slackSend channel: "$SLACK_CHANNEL", message: "Topo $env.TAG_NAME deployment waiting for confirmation on (<${env.RUN_DISPLAY_URL}|Jenkins>)"
                    }
                }
                stage('confirm') {
                    when { environment name: 'TAG_NAME', value: 'prod' }
                    steps {
                        input(message: "Are you sure you want to deploy on $env.TAG_NAME?")
                    }
                }
                stage('containerize') {
                    stages {
                        stage('login') {
                            steps {
                                withCredentials([[
                                    $class          : 'UsernamePasswordMultiBinding',
                                    credentialsId   : 'harbor-id',
                                    usernameVariable: 'HARBOR_USER',
                                    passwordVariable: 'HARBOR_PASS'
                                ]]) {
                                    sh '''
                                        docker login -u $HARBOR_USER -p $HARBOR_PASS https://$HARBOR_REGISTRY
                                    '''
                                }
                            }
                        }
                        stage('dji-decoder') {
                            steps {
                                lock("dji-flightlog-decoder") {
                                    sh '''
                                        NAME=flightlog-decoder
                                        DOCKER_NAME=dji-$NAME
                                        docker build . -f docker/build/$NAME/Dockerfile -t $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_TAG
                                        docker push $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_TAG
                                        if [ "$DOCKER_RELEASE_TAG" != "$DOCKER_TAG" ]; then
                                            docker tag $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_TAG $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_RELEASE_TAG
                                            docker push $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_RELEASE_TAG
                                            docker rmi $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_RELEASE_TAG
                                        fi
                                        docker rmi $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$DOCKER_TAG
                                    '''
                                }
                            }
                        }
                        stage('Deploy') {
                            when { not { environment name: 'DEPLOY_ENV', value: '' } }
                            steps {
                                sshagent(['JenkinsSSH']) {
                                sh '''
                                    scp ./docker/deploy/common-services.yml innovatm@$DEPLOY_SERVER:/opt/dji/flightlog-decoder/common-services.yml
                                    scp ./docker/deploy/docker-compose.$DEPLOY_ENV.yml innovatm@$DEPLOY_SERVER:/opt/dji/flightlog-decoder/docker-compose.yml
                                    ssh innovatm@$DEPLOY_SERVER "cd /opt/dji/flightlog-decoder && docker-compose pull"
                                    ssh innovatm@$DEPLOY_SERVER "cd /opt/dji/flightlog-decoder && docker-compose up -d"
                                '''
                                }
                            }
                            post {
                                success {
                                    slackSend channel: "$SLACK_CHANNEL", color: "good", message: "DJI Flightlog Decoder is successfully deployed on DroneKeeper ${env.DEPLOY_ENV} (<${env.RUN_DISPLAY_URL}|logs>)"
                                }
                                unstable {
                                    slackSend channel: "$SLACK_CHANNEL", color: "warning", message: "DJI Flightlog Decoder is successfully deployed on DroneKeeper ${env.DEPLOY_ENV} (<${env.RUN_DISPLAY_URL}|logs>)"
                                }
                                failure {
                                    slackSend channel: "$SLACK_CHANNEL", color: "danger", message: "ERROR: DJI Flightlog Decoder failed to deploy on DroneKeeper ${env.DEPLOY_ENV} (<${env.RUN_DISPLAY_URL}|logs>)"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    post {
        always {
            deleteDir()
        }
    }
}
