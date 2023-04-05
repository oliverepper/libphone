package de.oliver_epper.libphone;

import java.util.Scanner;
import java.util.concurrent.Executors;

public class Main {
    public static void main(String[] args) {
        var executor = Executors.newFixedThreadPool(1);

        var nameservers = new String[2];
        nameservers[0] = "217.237.148.22";
        nameservers[1] = "217.237.150.51";
        var stunservers = new String[1];
        stunservers[0] = "stun.t-online.de";

        final Phone phone;

        try {
            phone = new Phone("Java Cli ☕️", nameservers, stunservers);

            Runnable register = () -> phone.registerThread("Test");

            executor.submit(register);

            phone.registerOnIncomingCallIndexCallback((callIndex, ctx) -> {
                try {
                    System.out.println("Incoming call: " + callIndex + " - with id: " + phone.getCallId(callIndex));
                    var answerAfter = phone.answerAfter(callIndex);
                    if (answerAfter >= 0) {
                        Runnable answer = () -> {
                            try {
                                Thread.sleep(answerAfter);
                                phone.answer(callIndex);
                            } catch (PhoneException | InterruptedException e) {
                                e.printStackTrace();
                            }
                        };

                        executor.submit(answer);
                    } else {
                        // FIXME: start ringing
                    }
                } catch (PhoneException e) {
                    e.printStackTrace();
                }
            });

            phone.registerOnIncomingCallIdCallback((callId, ctx) -> {
                try {
                    System.out.println("Incoming call: " + callId + " - with index: " + phone.getCallIndex(callId));
                    var answerAfter = phone.answerAfter(callId);
                    if (answerAfter >= 0) {
                        Runnable answer = () -> {
                            try {
                                Thread.sleep(answerAfter);
                                phone.answer(callId);
                            } catch (PhoneException | InterruptedException e) {
                                e.printStackTrace();
                            }
                        };

                        executor.submit(answer);
                    } else {
                        // FIXME: start_ringing
                    }
                } catch (PhoneException e) {
                    e.printStackTrace();
                }
            });

            phone.registerOnCallStateIndexCallback((callIndex, state, ctx) ->
                    System.out.println("Call index: " + callIndex + " state: " + Phone.getState(state)));

            phone.registerOnCallStateIdCallback((callId, state, ctx) ->
                    System.out.println("Call id: " + callId + " state: " + Phone.getState(state)));

            phone.configureOpus(1, 8, 16000);
            phone.connect("tel.t-online.de", "+4965191899543", null);
            Phone.setLogLevel(0);
            // TODO: print libphone version

            var scanner = new Scanner(System.in);
            var command = '?';
            do {
                System.out.println("command: " + command);

                switch (command) {
                    case 'c' -> {
                        scanner.nextLine(); // flush stdin
                        System.out.print("Please enter number: ");
                        var number = scanner.nextLine();
                        try {
                            System.out.println("Calling: " + number);
                            phone.call(number);
                        } catch (PhoneException e) {
                            e.printStackTrace();
                        }
                    }
                    case 'C' -> phone.call("+491804100100");
                    case 'a' -> {
                        scanner.nextLine(); // flush stdin
                        System.out.print("Please enter call index: ");
                        var index = scanner.nextInt();
                        System.out.println("Answering call with index : " + index);
                        try {
                            phone.answer(index);
                        } catch (PhoneException e) {
                            e.printStackTrace();
                        }
                    }
                    case 'A' -> {
                        scanner.nextLine(); // flush stdin
                        System.out.print("Please enter call id: ");
                        var id = scanner.nextLine();
                        System.out.println("Answering call with id: " + id);
                        try {
                            phone.answer(id);
                        } catch (PhoneException e) {
                            e.printStackTrace();
                        }
                    }
                    case 'h' -> {
                        scanner.nextLine(); // flush stdin
                        System.out.print("Please enter call index: ");
                        var input = scanner.nextInt();
                        System.out.println("Hanging up call with index : " + input);
                        try {
                            phone.hangup(input);
                        } catch (PhoneException e) {
                            e.printStackTrace();
                        }
                    }
                    case 'H' -> {
                        scanner.nextLine(); // flush stdin
                        System.out.print("Please enter call index: ");
                        var input = scanner.nextLine();
                        System.out.println("Hanging up call with index : " + input);
                        try {
                            phone.hangup(input);
                        } catch (PhoneException e) {
                            e.printStackTrace();
                        }
                    }
                    case 'e' -> {
                        System.out.println("Hanging up all calls");
                        phone.hangupCalls();
                    }
                    case 'l' -> {
                        scanner.nextLine();
                        System.out.print("Please enter desired log-level: ");
                        var input = scanner.nextInt();
                        Phone.setLogLevel(input);
                    }
                    case 'd' -> {
                        System.out.println("""
                                1 - no filter [default, press enter]
                                2 - input devices
                                3 - output devices
                                """);
                        scanner.nextLine(); // flush stdin
                        System.out.print("Do you want a filter: ");
                        Phone.DeviceFilter filter;
                        var input = scanner.nextLine();
                        switch (input) {
                            case "1" -> filter = Phone.DeviceFilter.INPUT_DEVICES;
                            case "2" -> filter = Phone.DeviceFilter.OUPUT_DEVICES;
                            default -> filter = Phone.DeviceFilter.NONE;
                        }
                        for (Phone.AudioDeviceInfo info : Phone.getAudioDevices(filter)) {
                            System.out.println(info);
                        }
                        System.out.println();
                    }
                    case 'D' -> {
                        scanner.nextLine(); // flush stdin
                        System.out.print("Please enter desired capture device: ");
                        var captureDevice = scanner.nextInt();
                        System.out.print("Please enter desired playback device: ");
                        var playbackDevice = scanner.nextInt();
                        try {
                            Phone.setAudioDevices(captureDevice, playbackDevice);
                        } catch (PhoneException e) {
                            e.printStackTrace();
                        }
                    }
                    default -> System.out.println("""
                        c - call a number
                        C - call Time Announcement of Telekom Germany
                        a - answer a call (index)
                        A - answer a call (id)
                        h - hangup a call (index)
                        H - hangup a call (id)
                        e - kill all calls
                        l - change log level
                        d - list audio devices
                        D - change audio devices
                        q - quit
                        """);
                }

                command = scanner.next().charAt(0);
            } while (command != 'q');

            System.out.println("Shutting down...");
            phone.destroy();
            executor.shutdown();

        } catch (Throwable t) {
            t.printStackTrace();
            System.exit(1);
        }
    }
}