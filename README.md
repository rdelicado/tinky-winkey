<div align="center">

![Security](https://img.shields.io/badge/Seguridad-brown?style=for-the-badge)
![Service Keylogger](https://img.shields.io/badge/Servicio-Keylogger-blue?style=for-the-badge)
![C Language](https://img.shields.io/badge/Lenguaje-C-red?style=for-the-badge)

*Un keylogger de bajo nivel para Windows diseñado para ejecutarse como servicio persistente en segundo plano*

</div>

# Tinky Winkey

> Este proyecto es únicamente para fines educativos como parte del curriculum de 42 School. El código debe usarse solo en entornos controlados, como máquinas virtuales.  Usar keyloggers sin autorización apropiada es ilegal y poco ético.

## 🎯 Descripción

Este proyecto implementa un servicio de Windows llamado ```tinky``` y una aplicación keylogger llamada ```winkey```.  
Es un proyecto educativo de ```42 School``` diseñado para comprender la arquitectura de servicios de Windows, mecanismos de captura de teclado e interacción con procesos del sistema.

### 1. Servicio (svc.exe)

Es una aplicación de servicio de Windows con las siguientes capacidades:
- Instalación y registro como servicio del sistema
- Configuración de inicio automático
- Iniciar y detener el servicio manualmente
- Eliminación del servicio
- Suplantación de token SYSTEM
- Gestión del proceso keylogger

### 2. Keylogger (winkey.exe)

Una aplicación de registro de teclas que:
- Detecta procesos en primer plano
- Captura pulsaciones de teclas mediante hooks de teclado de bajo nivel
- Registra marcas de tiempo y títulos de ventanas
- Guarda los datos capturados en formato legible
- Soporta identificadores de configuración regional

## Uso

### Compilar

```
nmake
```

### Gestión del servicio

```bash
# Instalar el servicio
svc.exe install

# Iniciar el servicio
svc.exe start

# Configurar tipo de inicio
svc.exe config <mode>

# Estado del servicio y keylogger
svc.exe status

# Detener el servicio
svc.exe stop

# Eliminar el servicio
svc.exe delete
```

### Salida

El keylogger genera un archivo de log (winkey.log) con el siguiente formato:
```
[MARCA_TIEMPO] - 'TITULO_VENTANA'
PULSACIONES_TECLAS
```

## Aviso de Seguridad

Este proyecto se desarrolla exclusivamente con fines educativos como parte del curriculum de ```42 School``` para entender conceptos de programación de sistemas Windows. La implementación debe usarse únicamente en entornos aislados como máquinas virtuales.

## 📄 Licencia

Este proyecto está licenciado bajo la WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🌐 Desarrollado como parte del curriculum de 42 School 🌐**

*"Your secrets are safe... with me"*

</div># tinky-winkey
