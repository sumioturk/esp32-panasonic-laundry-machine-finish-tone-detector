const functions = require('firebase-functions');
const admin = require('firebase-admin')


// // Create and Deploy Your First Cloud Functions
// // https://firebase.google.com/docs/functions/write-firebase-functions
//
exports.laundry = functions.https.onRequest((request, response) => {
    //admin.messaging().sendToDevicea
    response.send("yo");
});
