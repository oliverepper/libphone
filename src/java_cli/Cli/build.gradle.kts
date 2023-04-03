plugins {
    id("java")
    application
}

group = "de.oliver_epper.libphone"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
}

dependencies {
    implementation("net.java.dev.jna:jna:5.13.0")
    testImplementation(platform("org.junit:junit-bom:5.9.1"))
    testImplementation("org.junit.jupiter:junit-jupiter")
}

tasks.test {
    useJUnitPlatform()
}

application {
    mainClass.set("de.oliver_epper.libphone.Main")
}

tasks.named<JavaExec>("run") {
    //Put as value with trailing slash: "/<build output or download path>/"
    systemProperty("native-libs", "")
    standardInput = System.`in`
}