# Heizbox Monorepo

This repository contains the source code for the Heizbox, a custom-built induction heater for Dynavap vaporizers. The project is a monorepo that includes the frontend, backend, and embedded firmware.

## Project Overview

The Heizbox project consists of three main components:

*   **Frontend**: A React-based web interface for controlling the induction heater and viewing statistics.
*   **Backend**: A Cloudflare Worker that provides an API for the frontend and the embedded device.
*   **Embedded**: The firmware for the ESP32 microcontroller that controls the induction heater.

## Architecture

The project is structured as a monorepo using NX and pnpm. The workspace is organized as follows:

*   `/apps/frontend`: The React frontend application.
*   `/apps/backend`: The Cloudflare Worker backend.
*   `/apps/esp32`: The PlatformIO project for the ESP32 firmware.
*   `/libs/ui`: A shared UI component library.
*   `/libs/types`: Shared TypeScript type definitions.
*   `/libs/utils`: Shared utility functions for the frontend and backend.

## Getting Started

To get started with the project, you will need to have Node.js and pnpm installed.

1.  Clone the repository:

    ```bash
    git clone https://github.com/your-username/heizbox.git
    ```

2.  Install the dependencies:

    ```bash
    pnpm install
    ```

3.  Start the development servers:

    ```bash
    pnpm dev
    ```

This will start the frontend and backend development servers in parallel.

## Frontend

The frontend is a React application built with Vite. It uses Tailwind CSS for styling and communicates with the backend API to control the induction heater.

*   **Development server**: `pnpm nx serve frontend`
*   **Build**: `pnpm nx build frontend`

## Backend

The backend is a Cloudflare Worker that provides a RESTful API for the frontend and the embedded device. It uses Hono as the web framework and Cloudflare D1 as the database.

*   **Development server**: `pnpm nx serve backend`
*   **Build**: `pnpm nx build backend`
*   **Deploy**: `wrangler deploy`

## Embedded

The embedded firmware is written in C++ using the PlatformIO ecosystem and the Arduino framework. It runs on an ESP32 microcontroller and controls the induction heater.

*   **Build**: `pio run`
*   **Upload**: `pio run --target upload`
*   **Monitor**: `pio device monitor`

### Prerequisites

1.  **Visual Studio Code** with the **PlatformIO IDE extension**.
2.  **WiFi Credentials**: A `credentials.h` file must be created in the `include/` directory with the following content:

    ```cpp
    #define WIFI_SSID "YourSSID"
    #define WIFI_PASSWORD "YourPassword"
    ```

## Deployment

The frontend and backend can be deployed to Cloudflare Pages and Cloudflare Workers, respectively. The embedded firmware can be uploaded to the ESP32 using PlatformIO.
