pipeline {

    agent any

    environment {
        project = 'LikeTaskManager'                               // "abis","FOS","IDSDisplay","KLI","kumovis"
                
 
        qmakepath = '/home/jk/Qt/6.5.3/gcc_64/bin/qmake'
        desktopfile_name = 'LikeTaskManager.desktop'
        executable_name = 'LikeTaskManager'
        appimage_name = 'LikeTaskManager'
        pro_file = 'LikeTaskManager.pro'
        core_count = '12'
    }


    stages {
       stage('make clean') {
           steps {
               	dir ("/home/jk/qtprojects/${project}/src"){
                   sh 'git fetch origin'  
	       }
		dir ("/home/jk/qtprojects/${project}/src") {
                   sh 'git reset --hard origin/qt6'
               }
		   
              	 dir ("/home/jk/qtprojects/${project}/appimgreleaseci") {
                   sh 'make clean -j${core_count}'
               }
		   


           }
       }
       stage('qmake') {
           steps {
               dir ("/home/jk/qtprojects/${project}/appimgreleaseci") {
                   sh '${qmakepath} /home/jk/qtprojects/${project}/src/${pro_file} -spec linux-g++ CONFIG+=qtquickcompiler'

               }
           }
       }
       stage('make') {
           steps {
               dir ("/home/jk/qtprojects/${project}/appimgreleaseci") {
                   sh 'make -j${core_count}'
               }
           }
       }
       stage('linuxdeployqt') {
           steps {
               dir ("/home/jk/qtprojects/${project}/appimgreleaseci") {
                   sh 'rm /home/jk/qtprojects/${project}/AppDir/usr/bin/${executable_name}' //FOS-Projekt?
                   sh 'cp ${executable_name} /home/jk/qtprojects/${project}/AppDir/usr/bin/'
               }
               dir ("/home/jk/qtprojects/${project}/appimgstageing") {
                   sh '/home/jk/stageing/linuxdeployqt-b62-x86_64.AppImage /home/jk/qtprojects/${project}/AppDir/usr/share/applications/${desktopfile_name}  -appimage -bundle-non-qt-libs -qmake=$qmakepath  -verbose=2 -updateinformation="zsync|https://appimgupdate.grapho-metronic.com/${appimage_name}-x86_64.AppImage.zsync"'               }
           }
            
       }
       stage('copy files') {
           steps {
              
		dir ("/home/jk/qtprojects/${project}/appimgreleaseci") {
                   sh 'ifconfig'
               }
               sh 'rsync /home/jk/qtprojects/${project}/appimgstageing/${appimage_name}-x86_64.AppImage fileupload@192.168.56.30:/var/www/data'
               sh 'rsync /home/jk/qtprojects/${project}/appimgstageing/${appimage_name}-x86_64.AppImage.zsync fileupload@192.168.56.30:/var/www/data'
 
           }
       }
    }
}


