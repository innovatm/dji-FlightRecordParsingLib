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
                    environment name : 'TAG_NAME', value : 'rwanda-dev'
                    environment name : 'TAG_NAME', value : 'rwanda-prod'
                    environment name : 'TAG_NAME', value : 'rwanda-staging'
                    environment name : 'TAG_NAME', value : 'thailand-prod'
                }
            }

            steps {
                script {
                    env.DOCKER_REPO = "dji-flighlog-decoder"
                    env.VERSION= 'dev';
                    if ( env.TAG_NAME == 'dev' ) {
                        env.VERSION= 'dev';
                        env.DOCKER_TAG = 'dev';
                        env.DOCKER_RELEASE_TAG = 'dev';
                        env.DEPLOY_ENV= 'dev';
                        env.DEPLOY_SERVER = "62.4.14.218";
                    } else if ( env.TAG_NAME == 'prod' ) {
                        env.VERSION= 'prod';
                        env.DOCKER_TAG = 'prod';
                        env.DOCKER_RELEASE_TAG=sh(returnStdout: true, script: '''
                            git tag -l --points-at HEAD| grep -v -E 'dev|prod|rwanda-dev|rwanda-staging|rwanda-prod|thailand-prod' | xargs -n2
                        ''');
                        env.DEPLOY_ENV= 'prod';
                        env.DEPLOY_SERVER = "62.210.28.140";
                    } else if ( env.TAG_NAME == 'rwanda-dev' ) {
                        env.VERSION= 'rwanda-dev';
                        env.DOCKER_TAG = 'rwanda-dev';
                        env.DOCKER_RELEASE_TAG = 'rwanda-dev';
                        env.DEPLOY_ENV= 'rwanda-dev';
                        env.DEPLOY_SERVER = "151.80.11.74";
                    } else if ( env.TAG_NAME == 'rwanda-staging' ) {
                        env.VERSION= 'rwanda-staging';
                        env.DOCKER_TAG = 'staging';
                        env.DOCKER_RELEASE_TAG = 'staging';
                        env.DOCKER_REPO = 'uspace-rwanda';
                    } else if ( env.TAG_NAME == 'rwanda-prod' ) {
                        env.VERSION= 'rwanda-prod';
                        env.DOCKER_TAG = 'prod';
                        env.DOCKER_RELEASE_TAG=sh(returnStdout: true, script: '''
                            git tag -l --points-at HEAD| grep -v -E 'dev|prod|rwanda-dev|rwanda-staging|rwanda-prod|thailand-prod' | xargs -n2
                        ''')
                        env.DOCKER_REPO = 'uspace-rwanda';
                    } else if ( env.TAG_NAME == 'thailand-prod' ) {
                        env.VERSION= 'thailand-prod';
                        env.DOCKER_TAG = 'thailand-prod';
                        env.DOCKER_RELEASE_TAG=sh(returnStdout: true, script: '''
                            git tag -l --points-at HEAD| grep -v -E 'dev|prod|rwanda-dev|rwanda-staging|rwanda-prod|thailand-prod' | xargs -n2
                        ''')
                        env.DEPLOY_ENV= 'thailand-prod';
                        env.DOCKER_REPO = 'uspace-thailand';
                        env.DEPLOY_SERVER = "161.246.157.110";
                    }
                    sh '''
                        if [ "$DOCKER_TAG" = "prod" ] && [ ! -n "$DOCKER_RELEASE_TAG" ]; then
                            echo "Prod (prod or rwanda-prod) tag found without release tag"
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
                                        docker build . -f docker/build/$NAME/Dockerfile -t $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$BUILD_ENV
                                        docker push $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$BUILD_ENV
                                        if [ "$RELEASE_VERSION" != "$BUILD_ENV" ]; then
                                            docker tag $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$BUILD_ENV $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$RELEASE_VERSION
                                            docker push $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$RELEASE_VERSION
                                            docker rmi $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$RELEASE_VERSION
                                        fi
                                        docker rmi $HARBOR_REGISTRY/$DOCKER_REPO/$DOCKER_NAME:$BUILD_ENV
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
                                    slackSend channel: "$SLACK_CHANNEL", color: "good", message: "DJI Flightlog Decoder is successfully deployed on DroneKeeper/|${env.DEPLOY_ENV} (<${env.RUN_DISPLAY_URL}|logs>)"
                                }
                                unstable {
                                    slackSend channel: "$SLACK_CHANNEL", color: "warning", message: "DJI Flightlog Decoder is successfully deployed on DroneKeeper/|${env.DEPLOY_ENV} (<${env.RUN_DISPLAY_URL}|logs>)"
                                }
                                failure {
                                    slackSend channel: "$SLACK_CHANNEL", color: "danger", message: "ERROR: DJI Flightlog Decoder failed to deploy on DroneKeeper/|${env.DEPLOY_ENV} (<${env.RUN_DISPLAY_URL}|logs>)"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
