
// Protect the Patient Dashboard
if (sessionStorage.getItem("loggedInRole") !== "patient") {
    window.location.replace("/patient-login.html");
}

// Get logged-in patient ID
const patientId = Number(
    sessionStorage.getItem("loggedInPatientId")
);

// Patient navigation
const patientNavButtons = document.querySelectorAll("[data-patient-page]");
const patientPages = document.querySelectorAll(".page");

const patientPageNames = {
    overview: "Overview",
    profile: "My Profile",
    doctor: "My Doctor",
    beds: "Bed Information",
    billing: "Billing"
};

function showPatientPage(page) {
    patientPages.forEach(section => {
        section.classList.toggle(
            "active",
            section.id === `page-${page}`
        );
    });

    document.querySelectorAll(".nav-link[data-patient-page]").forEach(button => {
        button.classList.toggle(
            "active",
            button.dataset.patientPage === page
        );
    });

    document.getElementById("pageTitle").textContent =
        patientPageNames[page] || "Overview";

    document.getElementById("sidebar").classList.remove("open");
}

// Navigation events
patientNavButtons.forEach(button => {
    button.addEventListener("click", () => {
        showPatientPage(button.dataset.patientPage);
    });
});

// Mobile sidebar
document.getElementById("mobileMenu").addEventListener("click", () => {
    document.getElementById("sidebar").classList.toggle("open");
});

// Today's date
document.getElementById("todayLabel").textContent =
    new Date().toLocaleDateString("en-IN", {
        day: "numeric",
        month: "short",
        year: "numeric"
    });

// Logout
document.getElementById("patientLogout").addEventListener("click", () => {
    sessionStorage.removeItem("loggedInRole");
    sessionStorage.removeItem("loggedInPatientId");

    window.location.href = "/patient-login.html";
});

// Helper to update HTML elements
function setText(id, value) {
    const element = document.getElementById(id);

    if (element) {
        element.textContent = value;
    }
}

// Format paise into rupees
function formatMoney(paise) {
    return "₹" + (paise / 100).toLocaleString("en-IN", {
        minimumFractionDigits: 2,
        maximumFractionDigits: 2
    });
}

// Fetch patient information
async function loadPatientDashboard() {
    try {
        if (!patientId || !Number.isInteger(patientId)) {
            throw new Error("Invalid patient login. Please log in again.");
        }

        // Fetch backend information
        const [patientsRes, doctorsRes, bedsRes] = await Promise.all([
            fetch("/api/patients"),
            fetch("/api/doctors"),
            fetch("/api/beds")
        ]);

        if (!patientsRes.ok || !doctorsRes.ok || !bedsRes.ok) {
            throw new Error("Could not connect to the hospital server.");
        }

        const [patients, doctors, beds] = await Promise.all([
            patientsRes.json(),
            doctorsRes.json(),
            bedsRes.json()
        ]);

        // Find the logged-in patient
        const patient = patients.find(p => p.id === patientId);

        if (!patient) {
            throw new Error(
                "Patient record not found. Please contact the administrator."
            );
        }

        // Find assigned doctor
        const doctor = doctors.find(d => d.id === patient.doctorId);

        // Find assigned bed
        const bed = beds.find(b => b.id === patient.bedId);

        // Fetch billing information
        let bill = null;

        const billRes = await fetch(`/api/bills/${patientId}`);

        if (billRes.ok) {
            const billData = await billRes.json();

            if (billData.success) {
                bill = billData;
            }
        }

        // Patient details
        setText("patientName", patient.name);
        setText("patientId", `#${patient.id}`);

        setText("detailName", patient.name);
        setText("detailId", `#${patient.id}`);
        setText("detailSpecialty", patient.specialty);

        setText("admissionStatus", patient.status);
        setText("profileStatus", patient.status);
        setText("bedAdmissionStatus", patient.status);

        // Admission date is not currently stored by the backend
        setText("admissionDate", "Not available");
        setText("profileAdmissionDate", "Not available");

        // Doctor information
        const doctorName = doctor ? doctor.name : "Not assigned";
        const doctorSpecialty = doctor ? doctor.specialty : "Not assigned";

        setText("doctorName", doctorName);
        setText("doctorSpecialty", doctorSpecialty);

        setText("profileDoctorName", doctorName);
        setText("profileDoctorSpecialty", doctorSpecialty);

        // Bed information
        const ward = bed ? bed.ward : "Not assigned";
        const bedNumber = bed ? bed.id : "Not assigned";

        setText(
            "bedNumber",
            bed ? `${ward} - ${bedNumber}` : "Not assigned"
        );

        setText("bedWard", ward);
        setText("bedDetailNumber", bedNumber);

        // Billing information
        if (bill) {
            const total = formatMoney(bill.totalPaise);
            const billStatus = bill.closed ? "Finalized" : "Open";

            setText("billAmount", total);
            setText("billingAmount", total);

            setText("paymentStatus", `${billStatus} bill`);
            setText("billingStatus", billStatus);
        } else {
            setText("billAmount", "No bill yet");
            setText("billingAmount", "No bill yet");

            setText("paymentStatus", "No bill available");
            setText("billingStatus", "No bill available");
        }

    } catch (error) {
        console.error("Patient dashboard error:", error);

        setText("patientName", "Unable to load patient");
        setText("patientId", "—");

        alert(error.message);
    }
}

// Load dashboard data
//loadPatientDashboard();

// Load immediately
loadPatientDashboard();

// Refresh patient information every 5 seconds
setInterval(loadPatientDashboard, 5000);