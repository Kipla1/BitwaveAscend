# Firebase cloud saves

Firebase is not connected yet. Local SQLite remains the save source until
Firebase Authentication and Firestore are configured.

## Set up a project

1. Create a Firebase project in the [Firebase console](https://console.firebase.google.com/).
2. In **Authentication → Sign-in method**, enable **Email/Password**.
3. In **Project settings → General**, copy the **Project ID** and **Web API key**.
4. Create a Cloud Firestore database.
5. Add rules that only let a signed-in user read and write their own save:

```text
match /users/{uid}/save/{document} {
  allow read, write: if request.auth != null && request.auth.uid == uid;
}
```

Never ship a service-account private key in the game. Firebase Auth ID tokens
should authorize each user's Firestore requests.

## Project changes still needed

The current account API uses username/password and local Argon2id hashes.
Firebase email/password auth needs an email address at registration and must
return an authenticated Firebase ID token. The frontend contract and local
user schema need that email field before Firebase sign-in can be connected.
Firebase's REST APIs use HTTPS for authentication and accept Firebase ID
tokens for Firestore access.

The current desktop build does not include a Firebase SDK or cloud transport.
Until those are wired, player progress saves locally in SQLite. See the
[Firebase C++ setup guide](https://firebase.google.com/docs/cpp/setup) before
choosing an SDK: its desktop support is beta and intended for development.
