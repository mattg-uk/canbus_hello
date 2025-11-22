# CAN bus HIL testing

## Project Goal

To demonstrate STM32-based CAN development within a CI/CD system that features HIL testing.

## HIL / CICD Project requirements 

This has three aspects, described below.

### CAN bus communications

The project software shall communicate simple messages from an STM32F103 board. The counter party will be a Raspberry Pi based test harness.

### Hardware In the Loop testing

Firstly, it shall be possible to test the code base by running unit tests on the STM32 hardware using ST-based or open-source tools, without reference to 3rd party proprietary applications.

Secondly, the unit tests shall and HIL interface shall operate such that the unit tests can be executed from a Jenkins based pipeline. 

### Jenkins build system

There shall be configurations for the software setup of a Jenkins based build server. This will be physically setup and interface to the Github repository.

The Jenkins instance shall execute a build pipeline:
 - activate upon a hook from Github or other configured repository
 - fetch the code from the repo
 - run static analysis of the code
 - build the code
 - and run unit tests on locally attached target hardware (HIL)
 - allow back reporting of build pass / fail where unit test file is pipeline fail.

The Jenkins instance shall make available builds images, coverage reports, and unit test reports.

The unit tests shall include 'live' communications to a Raspberry Pi based counterparty. This is intended to run on the Jenkins instance's host.
